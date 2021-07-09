#include "SpriteManager.h"
#include "load_texture.h"

SpriteManager::SpriteManager() :
	texture_atlas_manager_(NULL),
	sprite_texture_(NULL),
	scale_(0.f)
{
}

void SpriteManager::Init(gef::Vector2 sprite_pos, float scale, TextureAtlasManager* tex_atlas_manager, gef::Platform& platform)
{
	sprite_pos_ = sprite_pos;
	scale_ = scale;
	texture_atlas_manager_ = tex_atlas_manager;

	sprite_texture_ = CreateTextureFromPNG(tex_atlas_manager->GetTextureAtlas()->imagePath.c_str(), platform);
	sprite_.set_texture(sprite_texture_);
}

void SpriteManager::UpdateSprite(std::string part_name)
{
	SubTexture* frame = &texture_atlas_manager_->GetTextureAtlas()->subtextures[part_name];

	float sprite_x = frame->width * 0.5f - (frame->frameWidth * 0.5f + frame->frameX);
	float sprite_y = frame->height * 0.5f - (frame->frameHeight * 0.5f + frame->frameY);

	sprite_.set_width(frame->width);
	sprite_.set_height(frame->height);
	sprite_.set_uv_width(frame->width / texture_atlas_manager_->GetTextureAtlas()->width);
	sprite_.set_uv_height(frame->height / texture_atlas_manager_->GetTextureAtlas()->height);

	float u = frame->x / texture_atlas_manager_->GetTextureAtlas()->width;
	float v = frame->y / texture_atlas_manager_->GetTextureAtlas()->height;
	sprite_.set_uv_position(gef::Vector2(u, v));

	sprite_.set_position(gef::Vector4(sprite_x + sprite_pos_.x, sprite_y + sprite_pos_.y, 0.f));
}

void SpriteManager::RenderSprite(gef::SpriteRenderer* sprite_renderer_)
{
	sprite_renderer_->DrawSprite(sprite_);
}

void SpriteManager::RenderSprite(std::string part, gef::SpriteRenderer* sprite_renderer_)
{
	sprite_renderer_->DrawSprite(sprite_, texture_atlas_manager_->GetFinalTransform(part, scale_, sprite_pos_));
}
