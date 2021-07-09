#pragma once
#include <string>
#include <maths/matrix33.h>

#include "rapidjson\document.h"

class SubTexture
{
public:
	SubTexture();

	void ReadSubTextureFromJSON(const rapidjson::Value&);
	void BuildTransform();

	float x, y, width, height;
	float frameX, frameY, frameWidth, frameHeight;
	std::string name;
	gef::Matrix33 subtexture_transform_;
};

