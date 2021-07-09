#pragma once
#include "OutputNode.h"
#include <animation/skeleton.h>
#include <map>
#include <string>

class BlendTree
{
public:
	void Init(const gef::SkeletonPose& bind_pose);
	bool Update(float frame_time);

	OutputNode* GetOutputNode() { return output_node_; }

private:
	OutputNode* output_node_;
};

