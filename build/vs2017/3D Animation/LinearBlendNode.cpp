#include "LinearBlendNode.h"

LinearBlendNode::LinearBlendNode() :
	blend_(0.f)
{
}

bool LinearBlendNode::Process(float frame_time)
{
	if (inputs_.size() >= 2)
	{
		// blend the outputs of the 2 input nodes
		output_pose_.Linear2PoseBlend(inputs_[0]->GetOutput(), inputs_[1]->GetOutput(), blend_);
	}
	else
	{
		output_pose_ = bind_pose_;
	}

	return true;
}
