#pragma once
#include <bgfx/bgfx.h>

namespace FishEngine
{

enum class RenderViewType : bgfx::ViewId
{
	// Game
	Scene 		= 0,
	UI 			= 1,
	
	// Editor
	Editor 		= 2,
	SceneGizmos = 3,
	Picking 	= 4,
};
	
}
