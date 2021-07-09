#include "BlendTree.h"

void BlendTree::Init(const gef::SkeletonPose& bind_pose)
{
	output_node_ = new OutputNode();

	output_node_->SetBindPose(bind_pose);
}

bool BlendTree::Update(float frame_time)
{
	output_node_->Update(frame_time);

	return true;
}