#pragma once
#include "AnimatedMesh.h"
#include "BlendTree.h"
#include "ClipNode.h"
#include "LinearBlendNode.h"
#include <string>
#include "primitive_builder.h"
#include "primitive_renderer.h"

#include "btBulletDynamicsCommon.h"
#include "ragdoll.h"

class AnimationManager3D
{
public:
	AnimationManager3D(gef::Platform& platform);
	~AnimationManager3D();

	void Init(AnimatedMesh* animated_mesh);
	void Update(float frame_time);

	void CleanUpPhysicsWorld();

	void NextAnim();

	void SetBlendValue(float blend_val) { blend_val_ = blend_val; }
	float GetBlendValue() { return blend_val_; }

	int GetCurrentAnimNum() { return current_anim_; }

	void SetRagdollSimulating(bool rag_sim) { ragdoll_simulating_ = rag_sim; }
	bool IsRagdollSimulating() { return ragdoll_simulating_; }

	gef::MeshInstance GetFloor() { return floor_gfx_; }

private:
	gef::Animation* LoadAnimation(const char* anim_scene_filename, const char* anim_name);
	void BuildBlendTree();

	void InitRagdoll();

	void InitPhysicsWorld();
	void UpdatePhysicsWorld(float delta_time);

	void CreateRigidBodies();
	void CleanUpRigidBodies();
	void UpdateRigidBodies();

	gef::Platform& platform_;

	AnimatedMesh* animated_mesh_;

	std::string model_name_;

	BlendTree blend_tree_;
	ClipNode* start_clip_node_;
	ClipNode* end_clip_node_;
	LinearBlendNode* lin_blend_node_;

	PrimitiveBuilder* primitive_builder_;
	PrimitiveRenderer* primitive_renderer_;

	bool physics_world_active_;
	btDiscreteDynamicsWorld* dynamics_world_;
	btSequentialImpulseConstraintSolver* solver_;
	btBroadphaseInterface* overlapping_pair_cache_;
	btCollisionDispatcher* dispatcher_;
	btAlignedObjectArray<btCollisionShape*> collision_shapes_;

	gef::Mesh* floor_mesh_;
	gef::MeshInstance floor_gfx_;

	Ragdoll* ragdoll_;
	bool ragdoll_simulating_;

	float blend_val_;
	bool do_blend_;

	std::vector<gef::Animation*> anims_;
	int current_anim_;
	int next_anim_;
};

