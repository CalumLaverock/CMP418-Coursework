#include "OutputNode.h"

bool OutputNode::Process(float frame_time)
{
	if (inputs_.size() == 1)
	{
		output_pose_ = inputs_[0]->GetOutput();
	}
	else
	{
		output_pose_ = bind_pose_;
	}

	return true;
}