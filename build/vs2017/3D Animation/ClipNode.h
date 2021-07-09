#pragma once
#include "BlendNode.h"
#include <animation/animation.h>

class ClipNode : public BlendNode
{
public:
	ClipNode();

	const float GetAnimTime() const { return anim_time_; }
	void SetAnimTime(const float anim_time) { anim_time_ = anim_time; }

	const float GetPlaybackSpeed() const { return playback_speed_; }
	void SetPlaybackSpeed(const float playback_speed) { playback_speed_ = playback_speed; }

	const bool IsLooping() const { return looping_; }
	void SetLooping(const bool looping) { looping_ = looping; }

	const gef::Animation* GetClip() const { return clip_; }
	void SetClip(const gef::Animation* clip) { clip_ = clip; }

private:
	bool Process(float frame_time) override;

	/// A pointer to the animation clip to be sampled
	const gef::Animation* clip_;

	/// The current playback time the animation clip is being sampled at
	float anim_time_;

	/// The playback speed scaling factor used to adjust the speed the animation is played at
	float playback_speed_;

	/// The flag indicating whether the playback is to be looped or not
	bool looping_;
};

