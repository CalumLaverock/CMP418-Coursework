#include "BlendNode.h"

bool BlendNode::Update(float frame_time)
{
	// depth first, post order traversal
	bool all_valid_inputs = true;

	for (int i = 0; i < inputs_.size(); i++)
	{
		bool valid_input = false; 

		if(inputs_[i])
			valid_input = inputs_[i]->Update(frame_time);

		if (!valid_input)
			all_valid_inputs = false;
	}

	bool valid_output = false;

	if(all_valid_inputs)
		valid_output = Process(frame_time);

	return valid_output;
}

void BlendNode::SetInput(int input_num, BlendNode* input_node)
{
	if (inputs_.size() <= input_num)
	{
		inputs_.push_back(input_node);
	}
	else
	{
		inputs_[input_num] = input_node;
	}
}