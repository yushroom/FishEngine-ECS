#pragma once
#include "FishEngine/ECS.hpp"
#include <bgfx/bgfx.h>

namespace FishEngine
{

class RenderSystem : public ISystem
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
