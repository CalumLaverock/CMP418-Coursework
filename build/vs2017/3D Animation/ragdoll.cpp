#include "ragdoll.h"

#include <btBulletWorldImporter.h>
#include <system/debug_log.h>

Ragdoll::Ragdoll() :
	scale_factor_(1.0f)
{

}

void Ragdoll::Init(const gef::SkeletonPose& bind_pose, btDiscreteDynamicsWorld* dynamics_world, const char* physics_filename)
{
	bind_pose_ = bind_pose;
	pose_ = bind_pose;

	gef::Matrix44 identity;
	identity.SetIdentity();

	bone_rb_offset_matrices_.resize(bind_pose_.skeleton()->joint_count(), identity);
	bone_rbs_.resize(bind_pose_.skeleton()->joint_count(), NULL);
	bone_world_matrices_.resize(bind_pose_.skeleton()->joint_count());


	btBulletWorldImporter* fileLoader = new btBulletWorldImporter(dynamics_world);

	fileLoader->loadFile(physics_filename);

	int numRigidBodies = fileLoader->getNumRigidBodies();
	for (int i = 0; i < fileLoader->getNumRigidBodies(); i++)
	{
		btCollisionObject* obj = fileLoader->getRigidBodyByIndex(i);
		btRigidBody* body = btRigidBody::upcast(obj);

		// properties

		// The Blender object name
		std::string rb_name(fileLoader->getNameForPointer(body));

		// trim the blender rigid body name to match bone names
		rb_name = std::string(&rb_name.c_str()[sizeof("OBArmature_") - 1]);
		char* new_name = (char*)rb_name.c_str();
		new_name[rb_name.length() - sizeof("_hitbox") + 1] = 0;
		rb_name = std::string(new_name);

		gef::DebugOut("  object name = %s\n", rb_name.c_str());	
		gef::DebugOut("  get position = (%4.3f,%4.3f,%4.3f)\n",
			body->getCenterOfMassPosition().getX(),
			body->getCenterOfMassPosition().getY(),
			body->getCenterOfMassPosition().getZ());			// Blender CoM
		if (body->getInvMass() == 0)
			gef::DebugOut("  static object\n");
		else
		{
			gef::DebugOut("  mass = %4.3f\n", 1 / body->getInvMass());		// Blender Mass
		}
		gef::DebugOut("\n");

		if (bind_pose_.skeleton())
		{
			gef::StringId joint_name_id = gef::GetStringId(rb_name);
			if (joint_name_id != 0)
			{
				// find bone in skeleton that matches the name of the rigid body
				int joint_num = bind_pose_.skeleton()->FindJointIndex(joint_name_id);
				if (joint_num != -1)
				{
					bone_rbs_[joint_num] = body;

					//RB_WT
					btTransform rigid_body_world_transform = body->getCenterOfMassTransform();
					gef::Matrix44 rigid_body_world_transform_matrix = btTransform2Matrix(rigid_body_world_transform);

					//B_WT
					gef::Matrix44 bone_world_transform_matrix = bind_pose_.global_pose()[joint_num];
					gef::Vector4 bone_world_translation = bone_world_transform_matrix.GetTranslation();

					//scale B_WT to match animation system units
					bone_world_transform_matrix.SetTranslation(bone_world_translation * scale_factor_);

					//inv(B_WT)
					gef::Matrix44 inv_bone_world_transform_matrix;
					inv_bone_world_transform_matrix.AffineInverse(bone_world_transform_matrix);

					// RB_OFF = RB_WT * B_WT^-1
					bone_rb_offset_matrices_[joint_num] = rigid_body_world_transform_matrix * inv_bone_world_transform_matrix;
				}
			}
		}
	}

	delete fileLoader;
	fileLoader = NULL;
}

Ragdoll::~Ragdoll()
{
}

void Ragdoll::UpdatePoseFromRagdoll()
{
	for (int bone_num = 0; bone_num < bind_pose_.skeleton()->joint_count(); ++bone_num)
	{
		const gef::Joint& joint = bind_pose_.skeleton()->joint(bone_num);
		gef::Matrix44 anim_bone_local_transform;


		btRigidBody* bone_rb = bone_rbs_[bone_num];
		if (bone_rb)
		{
			//inv(RB_OFF)
			gef::Matrix44 inv_rigid_body_offset;
			inv_rigid_body_offset.AffineInverse(bone_rb_offset_matrices_[bone_num]);

			//RB_WT
			btTransform rigid_body_world_transform = bone_rb->getCenterOfMassTransform();
			gef::Matrix44 rigid_body_world_transform_matrix = btTransform2Matrix(rigid_body_world_transform);

			//inv(RB_OFF) * RB_WT (* P_WT, only if bone has parent else like multiplying by identity)
			gef::Matrix44 bone_world_transform_matrix = inv_rigid_body_offset * rigid_body_world_transform_matrix;

			//scale bone_world_transform by inverse of scale factor to match physics system units
			gef::Vector4 bone_world_translation = bone_world_transform_matrix.GetTranslation();
			float inv_scale_factor = 1.f / scale_factor_;
			bone_world_transform_matrix.SetTranslation(bone_world_translation * inv_scale_factor);

			if (joint.parent == -1)
			{
				//local bone transform = world bone transform if it has no parent, else multiply by parent
				//B_LT = inv(RB_OFF) * RB_WT = B_WT
				anim_bone_local_transform = bone_world_transform_matrix;
			}
			else
			{
				//inv(P_WT)
				gef::Matrix44 inv_parent_world_transform_matrix;
				inv_parent_world_transform_matrix.AffineInverse(bone_world_matrices_[joint.parent]);

				//B_LT = inv(RB_OFF) * RB_WT * inv(P_WT)
				anim_bone_local_transform = bone_world_transform_matrix * inv_parent_world_transform_matrix;
			}

		}
		else
		{
			anim_bone_local_transform = bind_pose_.local_pose()[bone_num].GetMatrix();
		}

		// calculate bone world transforms for anim skeleton
		if (joint.parent == -1)
		{
			bone_world_matrices_[bone_num] = anim_bone_local_transform;
		}
		else
		{
			bone_world_matrices_[bone_num] = anim_bone_local_transform * bone_world_matrices_[joint.parent];
		}
	}

	pose_.CalculateLocalPose(bone_world_matrices_);
	pose_.CalculateGlobalPose();
}

void Ragdoll::UpdateRagdollFromPose()
{
	for (int bone_num = 0; bone_num < bind_pose_.skeleton()->joint_count(); ++bone_num)
	{
		const gef::Joint& joint = bind_pose_.skeleton()->joint(bone_num);

		btRigidBody* bone_rb = bone_rbs_[bone_num];
		if (bone_rb)
		{
			//todo CALCULATE THE RIGID BODY WORLD TRANSFORM BASED ON THE CURRENT SKELETON POSE
			//RB_OFF
			gef::Matrix44 rigid_body_offset_matrix = bone_rb_offset_matrices_[bone_num];

			//B_WT
			gef::Matrix44 bone_world_transform_matrix = pose_.global_pose()[bone_num];

			//scale B_WT to match animation system units
			gef::Vector4 bone_world_translation = bone_world_transform_matrix.GetTranslation();
			bone_world_transform_matrix.SetTranslation(bone_world_translation * scale_factor_);

			//RB_WT = RB_OFF * B_WT
			gef::Matrix44 rigid_body_world_transform_matrix = rigid_body_offset_matrix * bone_world_transform_matrix;

			btTransform rigid_body_world_transform = Matrix2btTransform(rigid_body_world_transform_matrix);
			bone_rb->setCenterOfMassTransform(rigid_body_world_transform);
			bone_rb->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
			bone_rb->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
		}

	}

}

gef::Matrix44 btTransform2Matrix(const btTransform& transform)
{
	gef::Matrix44 result;

	btQuaternion rotation = transform.getRotation();
	btVector3 position = transform.getOrigin();

	result.Rotation(gef::Quaternion(rotation.x(), rotation.y(), rotation.z(), rotation.w()));
	result.SetTranslation(gef::Vector4(position.x(), position.y(), position.z()));

	return result;
}

btTransform Matrix2btTransform(const gef::Matrix44& mtx)
{
	gef::Vector4 mtx_position = mtx.GetTranslation();

	gef::Quaternion mtx_rot;
	mtx_rot.SetFromMatrix(mtx);

	btTransform result;
	result.setOrigin(btVector3(mtx_position.x(), mtx_position.y(), mtx_position.z()));
	result.setRotation(btQuaternion(mtx_rot.x, mtx_rot.y, mtx_rot.z, mtx_rot.w));

	return result;
}