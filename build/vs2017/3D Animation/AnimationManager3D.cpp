#include "AnimationManager3D.h"

AnimationManager3D::AnimationManager3D(gef::Platform& platform) :
	platform_(platform),
	animated_mesh_(NULL),
	start_clip_node_(NULL),
	end_clip_node_(NULL),
	primitive_builder_(NULL),
	primitive_renderer_(NULL),
	floor_mesh_(NULL),
	ragdoll_(NULL),
	ragdoll_simulating_(false),
	physics_world_active_(false),
	do_blend_(false),
	blend_val_(0.f),
	current_anim_(0),
	next_anim_(1)
{
}

AnimationManager3D::~AnimationManager3D()
{
	delete start_clip_node_;
	start_clip_node_ = NULL;

	delete end_clip_node_;
	end_clip_node_ = NULL;

	delete lin_blend_node_;
	lin_blend_node_ = NULL;

	delete ragdoll_;
	ragdoll_ = NULL;

	CleanUpRigidBodies();

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete primitive_renderer_;
	primitive_renderer_ = NULL;

	for (int i = 0; i < anims_.size(); i++)
	{
		delete anims_[i];
		anims_[i] = NULL;
	}
}

void AnimationManager3D::Init(AnimatedMesh* animated_mesh)
{
	animated_mesh_ = animated_mesh;
	
	model_name_ = animated_mesh_->GetModelName();

	// populate the vector of animations
	for (int i = 0; i < animated_mesh_->GetAnimNames().size(); i++)
	{
		std::string anim_name;
		anim_name = model_name_ + "/" + model_name_ + "@" + animated_mesh_->GetAnimNames()[i] + ".scn";

		anims_.emplace_back(LoadAnimation(anim_name.c_str(), ""));
	}

	primitive_builder_ = new PrimitiveBuilder(platform_);
	primitive_renderer_ = new PrimitiveRenderer(platform_);

	BuildBlendTree();

	// only intialise the physics world if it is not already initialised
	if(!physics_world_active_)
		InitPhysicsWorld();

	CreateRigidBodies();
	InitRagdoll();
}

void AnimationManager3D::Update(float frame_time)
{
	if (do_blend_)
	{
		// if blending, increase blend value until it reaches 1.f
		// then change the animation to the new animation
		if (blend_val_ < 1.f)
		{
			blend_val_ += frame_time;
		}
		else
		{
			do_blend_ = false;
			blend_val_ = 0.f;

			if (anims_[current_anim_])
			{
				// setup the two clip nodes with the current animation and the next animation respectively
				start_clip_node_->SetClip(anims_[current_anim_]);
				start_clip_node_->SetLooping(true);
				start_clip_node_->SetAnimTime(0.f);

				end_clip_node_->SetClip(anims_[next_anim_]);
				end_clip_node_->SetLooping(true);
				end_clip_node_->SetAnimTime(0.f);
			}
		}
	}

	lin_blend_node_->SetBlendValue(blend_val_);

	blend_tree_.Update(frame_time);

	if (animated_mesh_->GetPlayer())
	{
		// update the bone matrices that are used for rendering the character
		// from the newly updated pose in the anim player
		animated_mesh_->GetPlayer()->UpdateBoneMatrices(blend_tree_.GetOutputNode()->GetOutput());
	}

	if (ragdoll_simulating_)
	{
		UpdateRigidBodies();
	}

	UpdatePhysicsWorld(frame_time);

	if (animated_mesh_->GetPlayer() && ragdoll_)
	{
		// if ragdoll is active, update the animated mesh's bones with the ragdoll pose
		// else update the ragdoll to follow the animated mesh's pose
		if (ragdoll_simulating_)
		{
			ragdoll_->UpdatePoseFromRagdoll();
			animated_mesh_->GetPlayer()->UpdateBoneMatrices(ragdoll_->pose());
		}
		else
		{
			ragdoll_->set_pose(blend_tree_.GetOutputNode()->GetOutput());
			ragdoll_->UpdateRagdollFromPose();
		}
	}
}

gef::Animation* AnimationManager3D::LoadAnimation(const char* anim_scene_filename, const char* anim_name)
{
	gef::Animation* anim = NULL;

	gef::Scene anim_scene;
	if (anim_scene.ReadSceneFromFile(platform_, anim_scene_filename))
	{
		// if the animation name is specified then try and find the named anim
		// otherwise return the first animation if there is one
		std::map<gef::StringId, gef::Animation*>::const_iterator anim_node_iter;
		if (anim_name)
			anim_node_iter = anim_scene.animations.find(gef::GetStringId(anim_name));
		else
			anim_node_iter = anim_scene.animations.begin();

		if (anim_node_iter != anim_scene.animations.end())
			anim = new gef::Animation(*anim_node_iter->second);
	}

	return anim;
}

void AnimationManager3D::BuildBlendTree()
{
	start_clip_node_ = new ClipNode();
	end_clip_node_ = new ClipNode();
	lin_blend_node_ = new LinearBlendNode();

	if (animated_mesh_->GetPlayer())
	{
		// set up the blend tree by giving all nodes the bind pose
		blend_tree_.Init(animated_mesh_->GetPlayer()->bind_pose());
		start_clip_node_->SetBindPose(animated_mesh_->GetPlayer()->bind_pose());
		end_clip_node_->SetBindPose(animated_mesh_->GetPlayer()->bind_pose());
		lin_blend_node_->SetBindPose(animated_mesh_->GetPlayer()->bind_pose());

		// set defult blend value to 0.f
		lin_blend_node_->SetBlendValue(0.f);
	}

	// set the current animation to the first animation in the vector
	current_anim_ = 0;
	next_anim_ = 1;
	if (anims_[current_anim_])
	{
		start_clip_node_->SetClip(anims_[current_anim_]);
		start_clip_node_->SetLooping(true);
		start_clip_node_->SetAnimTime(0.f);

		// wrap around to the start of the animation list if the next anim goes out of bounds
		next_anim_ = next_anim_ >= anims_.size() ? 0 : next_anim_;

		end_clip_node_->SetClip(anims_[next_anim_]);
		end_clip_node_->SetLooping(true);
		end_clip_node_->SetAnimTime(0.f);
	}

	lin_blend_node_->SetInput(0, start_clip_node_);
	lin_blend_node_->SetInput(1, end_clip_node_);

	blend_tree_.GetOutputNode()->SetInput(0, lin_blend_node_);
}

void AnimationManager3D::NextAnim()
{
	do_blend_ = true;

	current_anim_++;
	if (current_anim_ >= anims_.size())
		current_anim_ = 0;

	// wrap around to the start of the animation list if the next anim goes out of bounds
	next_anim_ = (current_anim_ + 1) >= anims_.size() ? 0 : current_anim_ + 1;
}

void AnimationManager3D::InitRagdoll()
{
	if (animated_mesh_->GetPlayer()->bind_pose().skeleton())
	{
		ragdoll_ = new Ragdoll();
		ragdoll_->set_scale_factor(0.01f);

		std::string ragdoll_filename;
		ragdoll_filename = model_name_ + "/ragdoll.bullet";

		ragdoll_->Init(animated_mesh_->GetPlayer()->bind_pose(), dynamics_world_, ragdoll_filename.c_str());
	}

	ragdoll_simulating_ = false;
}

void AnimationManager3D::InitPhysicsWorld()
{
	physics_world_active_ = true;

	/// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	btDefaultCollisionConfiguration* collision_configuration = new btDefaultCollisionConfiguration();

	/// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collision_configuration);

	/// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	overlapping_pair_cache_ = new btDbvtBroadphase();

	/// the default constraint solver . For parallel processing you can use a different solver (see Extras / BulletMultiThreaded)
	solver_ = new btSequentialImpulseConstraintSolver;

	dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, overlapping_pair_cache_, solver_, collision_configuration);
	dynamics_world_->setGravity(btVector3(0, -9.8f, 0));
}

void AnimationManager3D::CleanUpPhysicsWorld()
{
	for (int i = dynamics_world_->getNumConstraints() - 1; i >= 0; i--)
	{
		btTypedConstraint* constraint = dynamics_world_->getConstraint(i);
		dynamics_world_->removeConstraint(constraint);
		delete constraint;
	}


	// remove the rigidbodies from the dynamics world and delete them
	for (int i = dynamics_world_->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamics_world_->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamics_world_->removeCollisionObject(obj);
		delete obj;
	}

	// delete collision shapes
	for (int j = 0; j < collision_shapes_.size(); j++)
	{
		btCollisionShape* shape = collision_shapes_[j];
		collision_shapes_[j] = 0;
		delete shape;
	}

	// delete dynamics world
	delete dynamics_world_;

	// delete solver
	delete solver_;

	// delete broadphase
	delete overlapping_pair_cache_;

	// delete dispatcher
	delete dispatcher_;

	dynamics_world_ = NULL;
	solver_ = NULL;
	overlapping_pair_cache_ = NULL;
	dispatcher_ = NULL;
}

void AnimationManager3D::UpdatePhysicsWorld(float delta_time)
{
	const btScalar simulation_time_step = 1.0f / 20.0f;
	const int max_sub_steps = 1;
	dynamics_world_->stepSimulation(simulation_time_step, max_sub_steps);
}

void AnimationManager3D::CreateRigidBodies()
{
	//the ground is a cube of side 100 at position y = 0.
	{
		btVector3 groundHalfExtents(btScalar(50.), btScalar(1.), btScalar(50.));
		btCollisionShape* groundShape = new btBoxShape(groundHalfExtents);

		collision_shapes_.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -groundHalfExtents.y(), 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamics_world_->addRigidBody(body);

		floor_mesh_ = primitive_builder_->CreateBoxMesh(gef::Vector4(groundHalfExtents.x(), groundHalfExtents.y(), groundHalfExtents.z()));
		floor_gfx_.set_mesh(floor_mesh_);
		floor_gfx_.set_transform(btTransform2Matrix(groundTransform));
	}
}

void AnimationManager3D::CleanUpRigidBodies()
{
	delete floor_mesh_;
	floor_mesh_ = NULL;
}

void AnimationManager3D::UpdateRigidBodies()
{
	// update any rigid bodies that move in here
}