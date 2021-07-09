#pragma once
#include <vector>
#include <string>
#include <map>
#include "SubTexture.h"

#include "rapidjson\document.h"

class TextureAtlas
{
public:
	TextureAtlas();

	void ReadTextureAtlasFromJSON(rapidjson::Document& doc);

	float width, height;
	std::string imagePath;
	std::string name;
	std::string type;
	std::map<std::string, SubTexture> subtextures;
};

