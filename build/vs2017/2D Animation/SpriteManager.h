#pragma once
#include <graphics/texture.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include "TextureAtlasManager.h"

class SpriteManager
{
public:
	SpriteManager();

	void Init(gef::Vector2 sprite_pos, float scale, TextureAtlasManager* tex_atlas_manager, gef::Platform& platform);

	void UpdateSprite(std::string part_name);
	void RenderSprite(gef::SpriteRenderer* sprite_renderer_);
	void RenderSprite(std::string part, gef::SpriteRenderer* sprite_renderer);
	gef::Sprite* GetSprite() { return &sprite_; }
	gef::Vector2 GetSpritePos() { return sprite_pos_; }

private:
	TextureAtlasManager* texture_atlas_manager_;
	gef::Texture* sprite_texture_;
	gef::Sprite sprite_;
	gef::Vector2 sprite_pos_;
	float scale_;
};

