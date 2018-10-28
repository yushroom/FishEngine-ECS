#pragma once

#include "../ClassDef.hpp"
#include "../ECS/Component.hpp"
#include "../Math/Matrix4x4.hpp"

//#include <bgfx/bgfx.h>

namespace FishEngine
{

	class Material;
	class Mesh;
	class Camera;


	class Skin : public NonCopyable
	{
		//COMPONENT(Skin);
	public:
		std::vector<Matrix4x4> inverseBindMatrices;
		GameObject* root = nullptr;
		std::vector<GameObject*> joints;
		std::string name;
	};


	class Renderable : public Component
	{
		COMPONENT(Renderable);
		
	//	void SetMaterial();
	//	void SetMaterial(int index);
		
		void OnDrawGizmosSelected() const override;

	public:
		//Material* material = nullptr;
		std::vector<Material*> m_Materials;
		Mesh* m_Mesh = nullptr;
		Skin* m_Skin = nullptr;
	};


	class Skybox : public Component
	{
		COMPONENT(Skybox);
	public:
		Material * m_SkyboxMaterial = nullptr;
	};


	enum class DrawCallType
	{
		Opaque,
		Transparent,
		Overlay,
	};


	struct DrawCall
	{
	//	int renderQueue = 0;
		DrawCallType type = DrawCallType::Opaque;
		int viewID = 0;
		int subMeshId = -1;
		Mesh* mesh = nullptr;
		Material* material = nullptr;
		Camera*	camera = nullptr;
	//	uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
		uint64_t state = 0;
		Matrix4x4 modelMatrix;
	};
	

}
