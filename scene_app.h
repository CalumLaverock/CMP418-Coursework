#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include <graphics/sprite.h>
#include <input/input_manager.h>
#include <graphics/renderer_3d.h>

#include "2D Animation/SpriteManager.h"
#include "2D Animation/TextureAtlasManager.h"
#include "2D Animation/AnimationManager2D.h"

#include "3D Animation/AnimatedMesh.h"
#include "3D Animation/AnimationManager3D.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
}


class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
private:
	bool HandleInput(float frame_time);

	void InitFont();
	void CleanUpFont();
	void DrawHUD();

	// 2D system functions
	void Init2D();
	void CleanUp2D();
	bool Update2D(float frame_time);
	bool HandleInput2D(float frame_time);
	void Render2D();

	// 3D system functions
	void Init3D();
	void CleanUp3D();
	bool Update3D(float frame_time);
	bool HandleInput3D(float frame_time);
	void Render3D();

	void SetupCamera();
	void SetupLights();

	bool animate_3d_;
	bool ske_anim_;
    
	gef::SpriteRenderer* sprite_renderer_;
	gef::Renderer3D* renderer_3d_;
	gef::Font* font_;
	gef::InputManager* input_manager_;

	//2D system variables
	TextureAtlasManager* texture_atlas_manager_;
	AnimationManager2D* animation_manager_2d_;
	SpriteManager* sprite_manager_;

	const char* tex_json;
	const char* ske_json;

	//3D system variables
	AnimatedMesh* player_;
	AnimationManager3D* animation_manager_3d_;

	float fps_;

	gef::Vector4 camera_eye_;
	gef::Vector4 camera_lookat_;
	gef::Vector4 camera_up_;
	float camera_fov_;
	float near_plane_;
	float far_plane_;

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();
};

#endif // _SCENE_APP_H
