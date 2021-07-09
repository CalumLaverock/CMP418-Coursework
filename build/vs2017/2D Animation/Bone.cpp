#include "Bone.h"

Bone::Bone() :
	length(0),
	x(0.f),
	y(0.f),
	rotation(0.f)
{
}

std::map<std::string, Bone> Bone::ReadBonesFromJSON(rapidjson::Document& ske_doc)
{
	std::map<std::string, Bone> bones;
	const rapidjson::Value& bone_list = ske_doc["armature"][0]["bone"];

	for (int i = 0; i < (int) bone_list.Size(); i++)
	{
		Bone* bone = new Bone();
		bone->name = bone_list[i]["name"].GetString();

		if (bone_list[i].HasMember("parent"))
			bone->parent = bone_list[i]["parent"].GetString();
		else
			bone->parent = "no_parent";

		if (bone_list[i].HasMember("transform"))
		{
			if (bone_list[i]["transform"].HasMember("x"))
				bone->x = bone_list[i]["transform"]["x"].GetFloat();

			if (bone_list[i]["transform"].HasMember("y"))
				bone->y = bone_list[i]["transform"]["y"].GetFloat();

			if (bone_list[i]["transform"].HasMember("skY"))
				bone->rotation = bone_list[i]["transform"]["skY"].GetFloat();
		}

		bone->BuildLocalTransform(nullptr);

		bones[bone->name] = *bone;
		delete bone;
	}

	BuildWorldTransforms(&bones);

	return bones;
}

void Bone::BuildLocalTransform(AnimationManager2D* anim_manager)
{
	float new_x = x;
	float new_y = y;
	float new_rot = rotation;

	gef::Matrix33 translate, rotate;
	translate.SetIdentity();
	rotate.SetIdentity();

	if (anim_manager != nullptr)
	{
		// set translation and rotation using bone x, y, rot and animation x, y, rot
		for (auto& bone_key : anim_manager->GetCurrentAnimation()->bone_keys)
		{
			if (bone_key.name == name)
			{
				for (auto translate_key = bone_key.translation_keys.begin(); translate_key != bone_key.translation_keys.end(); ++translate_key)
				{ 
					if (std::next(translate_key) != bone_key.translation_keys.end())
					{
						TranslateKey next_translate_key = *std::next(translate_key);

						if (anim_manager->GetCurrentFrameNum() >= translate_key->start_time && anim_manager->GetCurrentFrameNum() < next_translate_key.start_time)
						{
							translate_key->current_frame++;

							new_x += gef::Lerp(translate_key->x, next_translate_key.x, translate_key->lerp_inc);
							new_y += gef::Lerp(translate_key->y, next_translate_key.y, translate_key->lerp_inc);

							translate_key->lerp_inc = ((float)translate_key->current_frame / (float)translate_key->duration);
							if (translate_key->lerp_inc >= 1.f)
							{
								translate_key->lerp_inc = 0.f;
								translate_key->current_frame = 0;
							}

							// must break from translation key loop as it will loop through all translation keys and once it reaches the final key,
							// will go into the below else statement setting the new_x to translation_key->x, etc despite the current frame being on a different
							// translation key
							break;
						}
					}
					else
					{
						new_x += translate_key->x;
						new_y += translate_key->y;
					}
				}

				for (auto rotate_key = bone_key.rotation_keys.begin(); rotate_key != bone_key.rotation_keys.end(); ++rotate_key)
				{
					if (std::next(rotate_key) != bone_key.rotation_keys.end())
					{
						RotateKey next_rotate_key = *std::next(rotate_key);

						if (anim_manager->GetCurrentFrameNum() >= rotate_key->start_time && anim_manager->GetCurrentFrameNum() < next_rotate_key.start_time)
						{
							rotate_key->current_frame++;

							float angle_difference = next_rotate_key.rotation - rotate_key->rotation;

							angle_difference += angle_difference > 180 ? -360 : angle_difference < -180 ? 360 : 0;

							new_rot += rotate_key->rotation + (rotate_key->lerp_inc * angle_difference);

							rotate_key->lerp_inc = ((float)rotate_key->current_frame / (float)rotate_key->duration);
							if (rotate_key->lerp_inc >= 1)
							{
								rotate_key->lerp_inc = 0.f;
								rotate_key->current_frame = 0;
							}

							break;
						}
					}
					else
					{
						new_rot += rotate_key->rotation;
					}
				}
			}
		}
	}

	translate.SetTranslation(gef::Vector2(new_x, new_y));
	rotate.Rotate(gef::DegToRad(new_rot));

	local_transform_ = rotate * translate;
}

void Bone::BuildWorldTransforms(std::map<std::string, Bone>* bones)
{
	for (std::pair<std::string, Bone> bone : *bones)
	{
		if (bone.first != "root")
		{
			std::string parent_name = "";
			gef::Matrix33 transform;

			(*bones)[bone.first].world_transform_.SetIdentity();
			transform.SetIdentity();

			transform = bone.second.local_transform_;
			parent_name = bone.second.parent;

			while (parent_name != "root")
			{
				transform = transform * (*bones)[parent_name].local_transform_;
				parent_name = (*bones)[parent_name].parent;
			}

			(*bones)[bone.first].world_transform_ = transform;
		}
	}
}
