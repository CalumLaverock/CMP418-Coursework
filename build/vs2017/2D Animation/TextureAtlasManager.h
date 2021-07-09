#pragma once
#include <system/application.h>
#include <maths/vector2.h>
#include <graphics/sprite.h>
#include <input/input_manager.h>
#include <vector>
#include <maths/matrix33.h>
#include <map>

#include "rapidjson\document.h"
#include "TextureAtlas.h"
#include "SkinSlot.h"
#include "Bone.h"

class TextureAtlasManager
{
public:
	TextureAtlasManager();
	~TextureAtlasManager();

	void Init(const char* tex_doc, const char* ske_doc);

	TextureAtlas* GetTextureAtlas() { return texture_atlas_; }
	std::map<std::string, Bone> GetBones() { return bones_; }
	std::map<std::string, SkinSlot> GetSkinSlots() { return skin_slots_; }
	std::vector<std::string> GetPartOrder() { return part_order_; }

	gef::Matrix33 GetFinalTransform(std::string part, float scale, gef::Vector2 sprite_pos);

	void UpdateBoneTransforms(AnimationManager2D* anim_manager);

private:
	std::vector<std::string> ReadPartOrderFromJSON(rapidjson::Document& ske_doc);

	TextureAtlas* texture_atlas_;
	std::vector<std::string> part_order_;
	std::map<std::string, SkinSlot> skin_slots_;
	std::map<std::string, Bone> bones_;
};

