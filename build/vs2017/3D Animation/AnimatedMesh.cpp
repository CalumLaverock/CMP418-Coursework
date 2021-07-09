#include "AnimatedMesh.h"
#include <maths/math_utils.h>

AnimatedMesh::AnimatedMesh(gef::Platform& platform) :
	platform_(platform),
	mesh_(NULL),
	player_(NULL),
	model_scene_(NULL)
{
}

void AnimatedMesh::Init()
{
	model_name_ = "xbot";

	// create a new scene object and read in the data from the file
	// no meshes or materials are created yet
	// we're not making any assumptions about what the data may be loaded in for
	model_scene_ = new gef::Scene();

	std::string model_file = model_name_ + "/" + model_name_ + ".scn";
	model_scene_->ReadSceneFromFile(platform_, model_file.c_str());

	// we do want to render the data stored in the scene file so lets create the materials from the material data present in the scene file
	model_scene_->CreateMaterials(platform_);

	// if there is mesh data in the scene, create a mesh to draw from the first mesh
	mesh_ = GetFirstMesh(model_scene_);

	// get the first skeleton in the scene
	gef::Skeleton* skeleton = GetFirstSkeleton(model_scene_);

	if (skeleton)
	{
		player_ = new gef::SkinnedMeshInstance(*skeleton);
		player_->set_mesh(mesh_);
	}

	// anims
	anim_names_.push_back("idle");
	anim_names_.push_back("walking_inplace");
	anim_names_.push_back("running_inplace");
}

void AnimatedMesh::CleanUp()
{
	delete player_;
	player_ = NULL;

	delete mesh_;
	mesh_ = NULL;

	delete model_scene_;
	model_scene_ = NULL;
}

void AnimatedMesh::Update(float frame_time)
{
	// build a transformation matrix that will position the character
	// use this to move the player around, scale it, etc.
	if (player_)
	{
		gef::Matrix44 player_scale, player_rotation, player_translation;
		player_scale.SetIdentity();
		player_rotation.SetIdentity();
		player_translation.SetIdentity();

		player_scale.Scale(gef::Vector4(0.01f, 0.01f, 0.01f));
		player_rotation.RotationY(gef::DegToRad(0.f));
		player_translation.SetTranslation(gef::Vector4(0.f, 0.f, 0.f));

		player_->set_transform(player_scale * player_rotation * player_translation);
	}
}

void AnimatedMesh::Render(gef::Renderer3D* renderer_3d_)
{
	// draw the player, the pose is defined by the bone matrices
	if (player_)
		renderer_3d_->DrawSkinnedMesh(*player_, player_->bone_matrices());
}

gef::Skeleton* AnimatedMesh::GetFirstSkeleton(gef::Scene* scene)
{
	gef::Skeleton* skeleton = NULL;
	if (scene)
	{
		// check to see if there is a skeleton in the the scene file
		// if so, pull out the bind pose and create an array of matrices
		// that wil be used to store the bone transformations
		if (scene->skeletons.size() > 0)
			skeleton = scene->skeletons.front();
	}

	return skeleton;
}

gef::Mesh* AnimatedMesh::GetFirstMesh(gef::Scene* scene)
{
	gef::Mesh* mesh = NULL;

	if (scene)
	{
		// now check to see if there is any mesh data in the file, if so lets create a mesh from it
		if (scene->mesh_data.size() > 0)
			mesh = model_scene_->CreateMesh(platform_, scene->mesh_data.front());
	}

	return mesh;
}
