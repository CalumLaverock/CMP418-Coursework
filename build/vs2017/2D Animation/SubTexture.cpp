#include "SubTexture.h"

SubTexture::SubTexture() :
	x(0.f), 
	y(0.f),
	width(0.f),
	height(0.f),
	frameX(0.f),
	frameY(0.f),
	frameWidth(0.f),
	frameHeight(0.f)
{
}

void SubTexture::ReadSubTextureFromJSON(const rapidjson::Value& subtex_doc)
{
	name = subtex_doc["name"].GetString();

	x = subtex_doc["x"].GetFloat();
	y = subtex_doc["y"].GetFloat();
	width = subtex_doc["width"].GetFloat();
	height = subtex_doc["height"].GetFloat();

	if (subtex_doc.HasMember("frameX"))
		frameX = subtex_doc["frameX"].GetFloat();
	else
		frameX = 0;
	if (subtex_doc.HasMember("frameY"))
		frameY = subtex_doc["frameY"].GetFloat();
	else
		frameY = 0;
	if (subtex_doc.HasMember("frameWidth"))
		frameWidth = subtex_doc["frameWidth"].GetFloat();
	else
		frameWidth = subtex_doc["width"].GetFloat();;
	if (subtex_doc.HasMember("frameHeight"))
		frameHeight = subtex_doc["frameHeight"].GetFloat();
	else
		frameHeight = subtex_doc["height"].GetFloat();;

	BuildTransform();
}

void SubTexture::BuildTransform()
{
	gef::Matrix33 translate, scale;

	float sprite_x = width * 0.5f - (frameWidth * 0.5f + frameX);
	float sprite_y = height * 0.5f - (frameHeight * 0.5f + frameY);

	subtexture_transform_.SetIdentity();
	translate.SetIdentity();
	scale.SetIdentity();

	translate.SetTranslation(gef::Vector2(sprite_x, sprite_y));
	scale.Scale(gef::Vector2(width, height));

	subtexture_transform_ = scale * translate;
}