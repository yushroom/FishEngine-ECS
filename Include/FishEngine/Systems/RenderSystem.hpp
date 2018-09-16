#pragma once
#include "FishEngine/ECS.hpp"
#include <bgfx/bgfx.h>


class RenderSystem : public ECS::ISystem
{
	SYSTEM(RenderSystem);
public:
	void OnAdded() override;
	void Start() override;
	void Update() override {}
	void Resize(int width, int height);

	void Draw();
};
