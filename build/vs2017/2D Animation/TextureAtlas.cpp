#include "TextureAtlas.h"

TextureAtlas::TextureAtlas() :
	width(0.f), 
	height(0.f)
{
}

void TextureAtlas::ReadTextureAtlasFromJSON(rapidjson::Document& tex_document)
{
	name = tex_document["name"].GetString();
	imagePath = tex_document["imagePath"].GetString();
	width = tex_document["width"].GetFloat();
	height = tex_document["height"].GetFloat();

	const rapidjson::Value& subtexture_array = tex_document["SubTexture"];
	for (int subtex_num = 0; subtex_num < (int)subtexture_array.Size(); ++subtex_num)
	{
		SubTexture* subtexture = new SubTexture();
		subtexture->ReadSubTextureFromJSON(subtexture_array[subtex_num]);
		subtextures[subtexture->name] = *subtexture;
		delete subtexture;
	}
}