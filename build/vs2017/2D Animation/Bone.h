#pragma once
#include <string>
#include <map>
#include <maths/matrix33.h>
#include <maths/math_utils.h>
#include "AnimationManager2D.h"

#include "rapidjson/document.h"

class Bone
{
public:
	Bone();

	static std::map<std::string, Bone> ReadBonesFromJSON(rapidjson::Document& ske_doc);
	static void BuildWorldTransforms(std::map<std::string, Bone>* bones);
	void BuildLocalTransform(AnimationManager2D* anim_manager);

	int length;
	std::string name, parent;
	float x, y, rotation;

	gef::Matrix33 local_transform_;
	gef::Matrix33 world_transform_;
};

