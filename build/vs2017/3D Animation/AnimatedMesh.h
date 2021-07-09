#pragma once
#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>
#include <vector>
#include <graphics/skinned_mesh_instance.h>
#include <animation/animation.h>
#include <graphics/scene.h>
#include <system/platform.h>
#include <graphics/renderer_3d.h>

class AnimatedMesh
{
public:
	AnimatedMesh(gef::Platform& platform);

	void Init();
	void CleanUp();
	void Update(float frame_time);
	void Render(gef::Renderer3D* renderer_3d_);

	gef::SkinnedMeshInstance* GetPlayer() { return player_; }
	std::vector<std::string> GetAnimNames() { return anim_names_; }
	std::string GetModelName() { return model_name_; }

private:
	gef::Mesh* GetFirstMesh(gef::Scene* scene);
	gef::Skeleton* GetFirstSkeleton(gef::Scene* scene);

	gef::Platform& platform_;

	gef::Mesh* mesh_;
	gef::SkinnedMeshInstance* player_;

	gef::Scene* model_scene_;

	std::vector<std::string> anim_names_;
	std::string model_name_;
};

