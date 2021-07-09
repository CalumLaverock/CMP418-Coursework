#pragma once
#include <string>
#include <maths/matrix33.h>
#include <maths/math_utils.h>
#include <map>

#include "rapidjson/document.h"

class SkinSlot
{
public:
	SkinSlot();

	static std::map<std::string, SkinSlot> ReadSkinSlotsFromJSON(rapidjson::Document& ske_doc);
	void BuildOffsetTransform();

	std::string name;
	std::string part_name;
	float x, y, rotation;
	gef::Matrix33 sprite_offset_;
};

