#include "SkinSlot.h"

SkinSlot::SkinSlot() :
	x(0.f),
	y(0.f),
	rotation(0.f)
{
}

std::map<std::string, SkinSlot> SkinSlot::ReadSkinSlotsFromJSON(rapidjson::Document& ske_doc)
{
	std::map<std::string, SkinSlot> skin_slots;
	const rapidjson::Value& slots = ske_doc["armature"][0]["skin"][0]["slot"];

	for (int i = 0; i < (int)slots.Size(); i++)
	{
		SkinSlot* skin_slot = new SkinSlot();

		skin_slot->name = ske_doc["armature"][0]["skin"][0]["slot"][i]["name"].GetString();

		const rapidjson::Value& slot = ske_doc["armature"][0]["skin"][0]["slot"][i]["display"][0];

		skin_slot->part_name = slot["name"].GetString();

		if (slot.HasMember("transform"))
		{
			skin_slot->x = slot["transform"]["x"].GetFloat();
			skin_slot->y = slot["transform"]["y"].GetFloat();

			if (slot["transform"].HasMember("skY"))
				skin_slot->rotation = slot["transform"]["skY"].GetFloat();
		}

		skin_slot->BuildOffsetTransform();

		skin_slots[skin_slot->name] = *skin_slot;
		delete skin_slot;
	}

	return skin_slots;
}

void SkinSlot::BuildOffsetTransform()
{
	gef::Matrix33 translate, rotate;
	sprite_offset_.SetIdentity();
	translate.SetIdentity();
	rotate.SetIdentity();

	translate.SetTranslation(gef::Vector2(x, y));
	rotate.Rotate(gef::DegToRad(rotation));

	sprite_offset_ = rotate * translate;
}