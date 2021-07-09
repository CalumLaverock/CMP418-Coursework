#pragma once
#include "BlendNode.h"

class LinearBlendNode : public BlendNode
{
public:
	LinearBlendNode();

	void SetBlendValue(float blend_val) { blend_ = blend_val; }

private:
	bool Process(float frame_time) override;

	float blend_;
};

