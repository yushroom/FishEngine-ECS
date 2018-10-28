#pragma once
#include "../ECS/SingletonSystem.hpp"

namespace FishEngine
{

	class SingletonRenderState : public SingletonComponent
	{
		friend class Scene;
		friend class RenderSystem;
	public:
		uint64_t GetState() const { return m_State; }


	protected:
		SingletonRenderState();

	private:
//		bgfx::UniformHandle m_UniformLightDir;
//		bgfx::UniformHandle m_UniformCameraPos;
//		bgfx::UniformHandle m_UniformJointMatrix;
		uint64_t m_State = 0;

		bool m_EnableFrustumCulling = false;
	};


	inline SingletonRenderState::SingletonRenderState()
	{
//		m_UniformLightDir = bgfx::createUniform("lightDir", bgfx::UniformType::Vec4);
//		m_UniformCameraPos = bgfx::createUniform("CameraPos", bgfx::UniformType::Vec4);
//		m_UniformJointMatrix = bgfx::createUniform("u_jontMatrix", bgfx::UniformType::Vec4, 128);
	}

}
