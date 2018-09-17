#pragma once
#include "FishEngine/ECS/System.hpp"
#include <bgfx/bgfx.h>

namespace FishEngine
{

	class RenderSystem : public System
	{
		SYSTEM(RenderSystem);
	public:
		void OnAdded() override;
		void Start() override;
		void Update() override {}
		void Resize(int width, int height);

		void Draw();
	};

}
