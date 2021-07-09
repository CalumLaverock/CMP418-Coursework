#pragma once
#include <vector>
#include <animation/skeleton.h>

class BlendNode
{
public:
	bool Update(float frame_time);
	void SetInput(int input_num, BlendNode* input_node);

	void SetBindPose(const gef::SkeletonPose& bind) { bind_pose_ = bind; output_pose_ = bind_pose_; }
	gef::SkeletonPose GetOutput() { return output_pose_; }

protected:
	virtual bool Process(float frame_time) = 0;

	std::vector<BlendNode*> inputs_;
	gef::SkeletonPose output_pose_;

	gef::SkeletonPose bind_pose_;
};

