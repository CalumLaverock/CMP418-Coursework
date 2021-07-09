#include "TextureAtlasManager.h"
#include "load_json.h"
#include "load_texture.h"

TextureAtlasManager::TextureAtlasManager() :
	texture_atlas_(NULL)
{
}

TextureAtlasManager::~TextureAtlasManager()
{
	delete texture_atlas_;
	texture_atlas_ = NULL;
}

void TextureAtlasManager::Init(const char* tex_doc, const char* ske_doc)
{
	texture_atlas_ = new TextureAtlas();

	// load, parse, and read the json file for the texture atlas metadata and store
	// in TextureAtlas struct
	char* json_tex = LoadJSON(tex_doc);

	rapidjson::Document tex_document;
	tex_document.Parse(json_tex);
	texture_atlas_->ReadTextureAtlasFromJSON(tex_document);

	char* json_ske = LoadJSON(ske_doc);

	rapidjson::Document ske_document;
	ske_document.Parse(json_ske);

	// read the skin slot, bone, and part order information from skeleton json, this information is
	// only used for skeleton based animations
	part_order_ = ReadPartOrderFromJSON(ske_document);
	skin_slots_ = SkinSlot::ReadSkinSlotsFromJSON(ske_document);
	bones_ = Bone::ReadBonesFromJSON(ske_document);
}

void TextureAtlasManager::UpdateBoneTransforms(AnimationManager2D* anim_manager)
{
	// only update the bone transforms if the animation has moved to the next frame to prevent
	// needlessly updating transform information
	if (anim_manager->IsChangeAnimFrame())
	{
		for (auto& bone : bones_)
		{
			bone.second.BuildLocalTransform(anim_manager);
		}

		Bone::BuildWorldTransforms(&bones_);

		anim_manager->SetChangeAnimFrame(false);
	}
}

std::vector<std::string> TextureAtlasManager::ReadPartOrderFromJSON(rapidjson::Document& ske_doc)
{
	std::vector<std::string> parts;

	rapidjson::Value& slots = ske_doc["armature"][0]["slot"];

	for (int i = 0; i < (int)slots.Size(); i++)
	{
		std::string part_name = slots[i]["name"].GetString();
		parts.push_back(part_name);
	}

	return parts;
}

gef::Matrix33 TextureAtlasManager::GetFinalTransform(std::string part, float scale, gef::Vector2 sprite_pos)
{
	std::string frame_name;
	gef::Matrix33 local_transform, sprite_offset, bone_transform, rig_transform;

	rig_transform.SetIdentity();
	rig_transform.Scale(gef::Vector2(scale, scale));
	rig_transform.SetTranslation(gef::Vector2(sprite_pos.x, sprite_pos.y));

	frame_name = skin_slots_[part].part_name;

	local_transform = GetTextureAtlas()->subtextures[frame_name].subtexture_transform_;
	sprite_offset = GetSkinSlots()[part].sprite_offset_;
	bone_transform = GetBones()[part].world_transform_;

	return local_transform * sprite_offset * bone_transform * rig_transform;
}
