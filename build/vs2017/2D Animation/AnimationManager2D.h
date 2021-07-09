#pragma once
#include <string>
#include <vector>
#include <map>

#include "rapidjson\document.h"

struct TranslateKey {
	TranslateKey() :
		start_time(0),
		duration(0),
		current_frame(0),
		lerp_inc(0.f),
		x(0.f),
		y(0.f)
	{}

	int start_time;
	int duration;
	int current_frame;
	float lerp_inc;
	float x, y;
};

struct RotateKey {
	RotateKey() :
		start_time(0),
		duration(0),
		current_frame(0),
		lerp_inc(0.f),
		rotation(0.f)
	{}

	int start_time;
	int duration;
	int current_frame;
	float lerp_inc;
	float rotation;
};

struct BoneKey {
	std::string name;
	std::vector<TranslateKey> translation_keys;
	std::vector<RotateKey> rotation_keys;
};

struct Animation2D
{
	Animation2D() :
		duration(0)
	{}

	int duration;
	std::string name;
	std::vector<BoneKey> bone_keys;
	std::vector<std::string> frames_list;
};

class AnimationManager2D
{
public:
	AnimationManager2D();

	void Init(const char* ske_doc);
	void Update(float frame_time);

	std::string GetCurrentFrameName() { return current_frame_name_; }
	int GetCurrentFrameNum() { return current_frame_num_; }
	void SetCurrentAnimation(std::string anim) { current_anim_ = anim; }
	Animation2D* GetCurrentAnimation() { return &animations[current_anim_]; }
	std::string GetAnimationType() { return animation_type_; }
	bool IsChangeAnimFrame() { return change_anim_frame_; }
	void SetChangeAnimFrame(bool anim_bool) { change_anim_frame_ = anim_bool; }

	void NextAnimation(bool next_anim);

private:
	void ReadAnimationFromJSON(rapidjson::Document& ske_doc);

	std::map<std::string, Animation2D> animations;
	std::vector<std::string> animation_names;

	float animation_timer_ = 0.f;
	int frame_rate;
	std::string animation_type_;
	int current_frame_num_ = 0;
	std::string current_frame_name_{ "" };
	std::string current_anim_;
	bool change_anim_frame_{ true };
};

