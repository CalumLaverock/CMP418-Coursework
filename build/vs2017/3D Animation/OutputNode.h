#pragma once
#include "BlendNode.h"

class OutputNode : public BlendNode
{
private:
	bool Process(float frame_time) override;
};

