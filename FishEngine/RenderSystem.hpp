#pragma once
#include "ECS.hpp"
#include <bgfx/bgfx.h>

class SingletonRenderState : public SingletonComponent
{
	friend class Scene;
	friend class RenderSystem;
public:
	uint64_t GetState() const { return m_State; }


protected:
	SingletonRenderState();

private:
	bgfx::UniformHandle m_UniformLightDir;
	bgfx::UniformHandle m_UniformBaseColor;
	bgfx::UniformHandle m_UniformPBRParams;
	bgfx::UniformHandle m_UniformCameraPos;
	uint64_t m_State = 0;
};


class RenderSystem : public ISystem
{
public:
	void OnAdded() override;
	void Start() override;
	void Update() override;
	void Resize(int width, int height);
};
