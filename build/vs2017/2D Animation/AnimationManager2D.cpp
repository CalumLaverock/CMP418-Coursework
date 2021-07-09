#include "AnimationManager2D.h"
#include "load_json.h"
#include "Bone.h"

AnimationManager2D::AnimationManager2D() :
	animation_timer_(0.f),
	frame_rate(0),
	current_frame_num_(0),
	change_anim_frame_(true)
{
}

void AnimationManager2D::Init(const char* ske_doc)
{
	// load, parse, and read the json file for the animation data into the map of animations
	char* json_ske = LoadJSON(ske_doc);

	rapidjson::Document ske_document;
	ske_document.Parse(json_ske);
	ReadAnimationFromJSON(ske_document);

	current_frame_num_ = 0;
	animation_timer_ = 0.f;
	change_anim_frame_ = true;
}

void AnimationManager2D::Update(float frame_time)
{
	animation_timer_ += frame_time;
	if (animation_timer_ >= (1.f / frame_rate))
	{
		change_anim_frame_ = true;

		current_frame_num_++;
		animation_timer_ = 0;

		if (current_frame_num_ >= animations[current_anim_].duration)
		{
			current_frame_num_ = 0;
		}

		if (animations[current_anim_].bone_keys.size() == 0)
		{
			// if the animation has no bone keys it's a sprite animation so animate it accordingly by setting the current frame
			current_frame_name_ = animations[current_anim_].frames_list[current_frame_num_];
		}
	}
}

void AnimationManager2D::ReadAnimationFromJSON(rapidjson::Document& ske_document)
{
	animations.clear();
	animation_names.clear();

	frame_rate = ske_document["armature"][0]["frameRate"].GetInt();
	animation_type_ = ske_document["armature"][0]["type"].GetString();

	if (animation_type_ == "Armature")
	{
		rapidjson::Value& animation_list = ske_document["armature"][0]["animation"];
		for (int i = 0; i < animation_list.Size(); i++)
		{
			Animation2D* anim = new Animation2D();

			anim->name = animation_list[i]["name"].GetString();

			// if the animation has bone keys then it is a skeleton animation, populate the bone keys
			anim->duration = animation_list[i]["duration"].GetInt();

			// Read and store the bone keys information
			rapidjson::Value& bone_keys = animation_list[i]["bone"];
			for (int j = 0; j < bone_keys.Size(); j++)
			{
				BoneKey* bone_key = new BoneKey;

				bone_key->name = bone_keys[j]["name"].GetString();

				// Read and store the translate keys information
				if (bone_keys[j].HasMember("translateFrame"))
				{
					int start_time_inc = 0;

					rapidjson::Value& translate_keys = bone_keys[j]["translateFrame"];
					for (int k = 0; k < translate_keys.Size(); k++)
					{
						TranslateKey* trans_key = new TranslateKey;

						trans_key->current_frame = 0;
						trans_key->lerp_inc = 0.f;
						trans_key->duration = translate_keys[k]["duration"].GetInt();

						trans_key->start_time = start_time_inc;
						start_time_inc += trans_key->duration;

						if (translate_keys[k].HasMember("x"))
							trans_key->x = translate_keys[k]["x"].GetFloat();
						else
							trans_key->x = 0.f;
						if (translate_keys[k].HasMember("y"))
							trans_key->y = translate_keys[k]["y"].GetFloat();
						else
							trans_key->y = 0.f;

						bone_key->translation_keys.push_back(*trans_key);
						delete trans_key;
					}
				}

				// Read and store the rotate keys information
				if (bone_keys[j].HasMember("rotateFrame"))
				{
					int start_time_inc = 0;

					rapidjson::Value& rotate_keys = bone_keys[j]["rotateFrame"];
					for (int k = 0; k < rotate_keys.Size(); k++)
					{
						RotateKey* rot_key = new RotateKey;

						rot_key->current_frame = 0;
						rot_key->lerp_inc = 0.f;
						rot_key->duration = rotate_keys[k]["duration"].GetInt();

						rot_key->start_time = start_time_inc;
						start_time_inc += rot_key->duration;

						if (rotate_keys[k].HasMember("rotate"))
							rot_key->rotation = rotate_keys[k]["rotate"].GetFloat();
						else
							rot_key->rotation = 0.f;

						bone_key->rotation_keys.push_back(*rot_key);
						delete rot_key;
						
					}
				}

				anim->bone_keys.push_back(*bone_key);
				delete bone_key;
			}

			animations[anim->name] = (*anim);
			animation_names.push_back(anim->name);
			delete anim;
		}
	}
	// populate the frame list if the animation is a sprite animation i.e. the animation has no bone keys
	else if(animation_type_ == "Sheet")
	{
		const rapidjson::Value& anims = ske_document["armature"];

		for (int i = 0; i < (int)anims.Size(); i++)
		{
			Animation2D* anim = new Animation2D;

			anim->name = anims[i]["animation"][0]["name"].GetString();
			anim->duration = anims[i]["animation"][0]["duration"].GetInt();

			const rapidjson::Value& frames = ske_document["armature"][i]["skin"][0]["slot"][0]["display"];
			for (int j = 0; j < (int)frames.Size(); j++)
			{
				std::string frame_name = frames[j]["name"].GetString();
				anim->frames_list.push_back(frame_name);
			}

			animations[anim->name] = *anim;
			animation_names.push_back(anim->name);
			delete anim;
		}
	}

	current_anim_ = ske_document["armature"][0]["defaultActions"][0]["gotoAndPlay"].GetString(); 
}

void AnimationManager2D::NextAnimation(bool next_anim)
{
	auto current_animation_iterator = std::find(animation_names.begin(), animation_names.end(), current_anim_);

	// reset the translate and rotate keys for all bones of the current (pre-change) animation
	for (auto& bone_key : animations[current_anim_].bone_keys)
	{
		for (auto& trans_key : bone_key.translation_keys)
		{
			trans_key.current_frame = 0;
			trans_key.lerp_inc = 0.f;
		}

		for (auto& rot_key : bone_key.rotation_keys)
		{
			rot_key.current_frame = 0;
			rot_key.lerp_inc = 0.f;
		}
	}

	// change the current animation to the next one in the list, forwards if next_anim = true, else backwards if next_anim = false
	if (next_anim)
	{
		if (std::next(current_animation_iterator) != animation_names.end())
		{
			current_anim_ = *std::next(current_animation_iterator);
		}
		// handle edge case if current animation is at end of list
		else
		{
			current_anim_ = *animation_names.begin();
		}
	}
	else
	{
		if (current_animation_iterator != animation_names.begin())
		{
			current_anim_ = *std::prev(current_animation_iterator);
		}
		// handle edge case if current animation is at start of list
		else
		{
			current_anim_ = *std::prev(animation_names.end());
		}
	}

	// reset the current frame to the start of the animation
	current_frame_num_ = 0;
	animation_timer_ = 0.f;
	change_anim_frame_ = true;
}