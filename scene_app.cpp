#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <input/keyboard.h>
#include <graphics/sprite.h>
#include "load_texture.h"

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	input_manager_(NULL),
	texture_atlas_manager_(NULL),
	animation_manager_2d_(NULL),
	player_(NULL),
	animation_manager_3d_(NULL),
	sprite_manager_(NULL),
	font_(NULL),
	animate_3d_(false),
	ske_anim_(false),
	tex_json("boy-anim_tex.json"),
	ske_json("boy-anim_ske.json")
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	renderer_3d_ = gef::Renderer3D::Create(platform_);
	input_manager_ = gef::InputManager::Create(platform_);

	InitFont();

	// default to 2D animation system
	Init2D();
	Init3D();
}

void SceneApp::Init2D()
{
	// initialise objects for 2D animation system
	texture_atlas_manager_ = new TextureAtlasManager();
	animation_manager_2d_ = new AnimationManager2D();
	sprite_manager_ = new SpriteManager();

	// defaults to sprite animation
	texture_atlas_manager_->Init(tex_json, ske_json);
	animation_manager_2d_->Init(ske_json);
	sprite_manager_->Init(gef::Vector2(platform_.width() * 0.5f, platform_.height() * 0.5f), 0.5f, texture_atlas_manager_, platform_);
}

void SceneApp::Init3D()
{
	// initialise objects for 3D animation system
	player_ = new AnimatedMesh(platform_);
	animation_manager_3d_ = new AnimationManager3D(platform_);
	
	player_->Init();
	animation_manager_3d_->Init(player_);

	SetupCamera();
	SetupLights();
}

void SceneApp::CleanUp()
{
	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	CleanUp2D();

	// only clean up the physics world if 3D system has been initialised and only
	// clean up physics world when the program closes rather than when switching to 2D system
	if(animation_manager_3d_)
		animation_manager_3d_->CleanUpPhysicsWorld();

	CleanUp3D();
}

void SceneApp::CleanUp2D()
{
	delete texture_atlas_manager_;
	texture_atlas_manager_ = NULL;

	delete animation_manager_2d_;
	animation_manager_2d_ = NULL;

	delete sprite_manager_;
	sprite_manager_ = NULL;
}

void SceneApp::CleanUp3D()
{
	delete player_;
	player_ = NULL;

	delete animation_manager_3d_;
	animation_manager_3d_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	HandleInput(frame_time);

	// only update the currently running system
	bool success = animate_3d_ ? Update3D(frame_time) : Update2D(frame_time);

	return success;
}
bool SceneApp::HandleInput(float frame_time)
{
	// read input devices
	if (input_manager_)
	{
		input_manager_->Update();

		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE))
			{
				// toggle between 3D and 2D system
				animate_3d_ = !animate_3d_;
			}
		}
	}

	return true;
}

bool SceneApp::Update2D(float frame_time)
{
	fps_ = 1.0f / frame_time;

	HandleInput2D(frame_time);
	
	animation_manager_2d_->Update(frame_time);

	texture_atlas_manager_->UpdateBoneTransforms(animation_manager_2d_);

	return true;
}
bool SceneApp::HandleInput2D(float frame_time)
{
	if (input_manager_)
	{
		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT))
			{
				animation_manager_2d_->NextAnimation(true);
			}
			else if (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT))
			{
				animation_manager_2d_->NextAnimation(false);
			}

			if (keyboard->IsKeyPressed(gef::Keyboard::KC_C))
			{
				// toggle between skeleton and sprite based animation
				ske_anim_ = !ske_anim_;

				// change skeleton and texture documents depending on type of animation
				if (ske_anim_)
				{
					tex_json = "Dragon_tex.json";
					ske_json = "Dragon_ske.json";
				}
				else
				{
					tex_json = "boy-anim_tex.json";
					ske_json = "boy-anim_ske.json";
				}

				// restart the 2D animation system using the new animation type
				CleanUp2D();
				Init2D();
			}
		}
	}

	return true;
}

bool SceneApp::Update3D(float frame_time)
{
	fps_ = 1.0f / frame_time;

	HandleInput3D(frame_time);

	player_->Update(frame_time);
	animation_manager_3d_->Update(frame_time);

	return true;
}
bool SceneApp::HandleInput3D(float frame_time)
{
	// read input devices
	if (input_manager_)
	{
		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT))
			{
				// progress to the next animation
				animation_manager_3d_->NextAnim();
			}

			if (keyboard->IsKeyDown(gef::Keyboard::KC_UP))
			{
				// slowly increase the blend value to blend between current and next animation
				animation_manager_3d_->SetBlendValue(animation_manager_3d_->GetBlendValue() + (frame_time / 5.f));

				if (animation_manager_3d_->GetBlendValue() >= 1.f)
				{
					animation_manager_3d_->NextAnim();
				}
			}
			else if (keyboard->IsKeyDown(gef::Keyboard::KC_DOWN))
			{
				// slowly decrease the blend value and cap it at 0.f
				animation_manager_3d_->SetBlendValue(animation_manager_3d_->GetBlendValue() - (frame_time / 5.f));

				if (animation_manager_3d_->GetBlendValue() < 0.f)
				{
					animation_manager_3d_->SetBlendValue(0.f);
				}
			}

			if (keyboard->IsKeyPressed(gef::Keyboard::KC_C))
			{
				// activate the ragdoll
				animation_manager_3d_->SetRagdollSimulating(!animation_manager_3d_->IsRagdollSimulating());
			}
		}
	}

	return true;
}

void SceneApp::Render()
{
	if (animate_3d_)
	{
		Render3D();
	}
	else
	{
		Render2D();
	}
}

void SceneApp::Render2D()
{
	sprite_renderer_->Begin();

	if (animation_manager_2d_->GetAnimationType() == "Armature")
	{
		// display a skeleton based animation
		for (auto& part : texture_atlas_manager_->GetPartOrder())
		{
			sprite_manager_->UpdateSprite(texture_atlas_manager_->GetSkinSlots()[part].part_name);
			sprite_manager_->RenderSprite(part, sprite_renderer_);
		}
	}
	else if (animation_manager_2d_->GetAnimationType() == "Sheet")
	{
		// display a sprite based animation
		sprite_manager_->UpdateSprite(animation_manager_2d_->GetCurrentFrameName());
		sprite_manager_->RenderSprite(sprite_renderer_);
	}

	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::Render3D()
{
	// setup view and projection matrices
	gef::Matrix44 projection_matrix;
	gef::Matrix44 view_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(camera_fov_, (float)platform_.width() / (float)platform_.height(), near_plane_, far_plane_);
	view_matrix.LookAt(camera_eye_, camera_lookat_, camera_up_);
	renderer_3d_->set_projection_matrix(projection_matrix);
	renderer_3d_->set_view_matrix(view_matrix);

	// draw meshes here
	renderer_3d_->Begin();

	player_->Render(renderer_3d_);

	renderer_3d_->DrawMesh(animation_manager_3d_->GetFloor());

	renderer_3d_->End();

	// setup the sprite renderer, but don't clear the frame buffer
	// draw 2D sprites here
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// change colour of 2D/3D text based on whether animation system is currently 2D or	3D
		int colour_3d = animate_3d_ ? 0xff00ff00 : 0xff0000ff;
		int colour_2d = !animate_3d_ ? 0xff00ff00 : 0xff0000ff;

		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
		font_->RenderText(sprite_renderer_, gef::Vector4(10.f, 0.f, -0.9f), 1.f, 0xffffffff, gef::TJ_LEFT, "Current Animation:");

		if (animate_3d_)
		{
			// change text between ON/OFF and colour of text based on if ragdoll is currently siumlated or not
			std::string ragdoll_text_toggle = animation_manager_3d_->IsRagdollSimulating() ? "ON" : "OFF";
			int ragdoll_text_colour = animation_manager_3d_->IsRagdollSimulating() ? 0xff00ff00 : 0xff0000ff;

			// display current animation name
			font_->RenderText(sprite_renderer_, gef::Vector4(220.f, 0.f, -0.9f), 1.f, 0xff0000ff, gef::TJ_LEFT, player_->GetAnimNames()[animation_manager_3d_->GetCurrentAnimNum()].c_str());
			
			// display the ragdoll simulation state
			font_->RenderText(sprite_renderer_, gef::Vector4(10.f, 25.f, -0.9f), 1.f, 0xffffffff, gef::TJ_LEFT, "Ragdoll:");
			font_->RenderText(sprite_renderer_, gef::Vector4(100.f, 25.f, -0.9f), 1.f, ragdoll_text_colour, gef::TJ_LEFT, ragdoll_text_toggle.c_str());

			// display the blend value
			font_->RenderText(sprite_renderer_, gef::Vector4(10.f, 50.f, -0.9f), 1.f, 0xffffffff, gef::TJ_LEFT, "Blend Value:");
			font_->RenderText(sprite_renderer_, gef::Vector4(145.f, 50.f, -0.9f), 1.f, 0xff00ff00, gef::TJ_LEFT, "%.3f", animation_manager_3d_->GetBlendValue());
		}
		else
		{
			// display current animation name
			font_->RenderText(sprite_renderer_, gef::Vector4(220.f, 0.f, -0.9f), 1.f, 0xff0000ff, gef::TJ_LEFT, animation_manager_2d_->GetCurrentAnimation()->name.c_str());
		}

		// display the animation system currently running, highlight the system in green that is running
		font_->RenderText(sprite_renderer_, gef::Vector4(660.f, 0.f, -0.9f), 1.f, 0xffffffff, gef::TJ_LEFT, "Animation System:");
		font_->RenderText(sprite_renderer_, gef::Vector4(870.f, 0.f, -0.9f), 1.f, colour_2d, gef::TJ_LEFT, "2D");
		font_->RenderText(sprite_renderer_, gef::Vector4(905.f, 0.f, -0.9f), 1.f, 0xffffffff, gef::TJ_LEFT, "|");
		font_->RenderText(sprite_renderer_, gef::Vector4(920.f, 0.f, -0.9f), 1.f, colour_3d, gef::TJ_LEFT, "3D");
	}
}

void SceneApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::SetupCamera()
{
	// initialise the camera settings
	camera_eye_ = gef::Vector4(-1.0f, 1.0f, 4.0f);
	camera_lookat_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_up_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_fov_ = gef::DegToRad(45.0f);
	near_plane_ = 0.01f;
	far_plane_ = 1000.f;
}

