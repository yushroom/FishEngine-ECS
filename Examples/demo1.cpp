#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Texture.hpp>

#include <imgui/imgui.h>

union Params
{
	struct { float m_glossiness, m_reflectivity, m_exposure, m_bgType; };
	Vector4 floats;
	
	Params() : floats() {}
};


class ModelViewer : public GameApp
{
public:
	void Start() override
	{

		//m_Shader = ShaderUtil::Compile("/Users/yushroom/program/FishEngine-ECS/shader/vs.bin", "/Users/yushroom/program/FishEngine-ECS/shader/fs.bin");
		{
			auto vs = "/Users/yushroom/program/FishEngine-ECS/Shaders/runtime/PBR_vs.bin";
			auto fs = "/Users/yushroom/program/FishEngine-ECS/Shaders/runtime/PBR_fs.bin";
			m_Shader = ShaderUtil::Compile(vs, fs);
		}

		{
			auto vs = "/Users/yushroom/program/FishEngine-ECS/Shaders/runtime/Skybox_vs.bin";
			auto fs = "/Users/yushroom/program/FishEngine-ECS/Shaders/runtime/Skybox_fs.bin";
			m_SkyboxShader = ShaderUtil::Compile(vs, fs);
		}

		{
			const char* path = "/Users/yushroom/program/FishEngine-ECS/Assets/Textures/uffizi_cross_128_filtered.dds";
			m_tex_filtered = TextureUtils::LoadTexture(path, BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_W_CLAMP);
		}
//		{
//			const char* path = "/Users/yushroom/program/FishEngine/Example/PBR/Assets/uffizi_cross.dds";
//			m_tex = TextureUtils::LoadTexture(path, BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_W_CLAMP);
//		}
//			const char* filePath = "/Users/yushroom/program/github/bgfx/examples/runtime/textures/bolonga_irr.dds";
//			m_texIrr = TextureUtils::LoadTexture(filePath, BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_W_CLAMP);

		m_params.m_exposure = 0.0f;
		m_params.m_bgType = 1.0f;

		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(goID);
			auto go = m_Scene->GetGameObjectByID(goID);
			go->GetTransform()->position.Set(0, 0, -15);
			m_Scene->GameObjectAddComponent<FreeCamera>(goID);
			Skybox* skybox = m_Scene->GameObjectAddComponent<Skybox>(goID);
			Material* skyboxMat = new Material();
			skyboxMat->SetVector("u_params", m_params.floats);
			skyboxMat->SetTexture("s_texCube", m_tex_filtered);
//			skyboxMat->SetTexture("s_texCubeIrr", m_texIrr);
			skyboxMat->SetShader(m_SkyboxShader);
			skybox->m_skyboxMaterial = skyboxMat;
		}
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(goID);
		}
		

		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				ECS::EntityID goID = m_Scene->CreateGameObject();
				auto go = m_Scene->GetGameObjectByID(goID);
				auto& pos = go->GetTransform()->position;
				pos.x = -7.5f + x * 1.5f;
				pos.y = -7.5f + y * 1.5f;
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
				rend->mesh = Mesh::Sphere;

				Material* mat = new Material();
				mat->SetShader(m_Shader);
				Vector4 pbrparams(x*0.1f, y*0.1f, 0, 0);
				mat->SetVector("BaseColor", Vector4::one);
				mat->SetVector("PBRParams", pbrparams);
				mat->SetTexture("AmbientCubemap", m_tex_filtered);
				rend->material = mat;
			}
		}
		
		m_Scene->AddSystem(new FreeCameraSystem());
	}
	
	void Update() override
	{
	}

private:
	Shader* m_Shader = nullptr;
	Shader* m_SkyboxShader = nullptr;
//	bgfx::TextureHandle m_tex;
	bgfx::TextureHandle m_tex_filtered;
	Params m_params;
};


int main(void)
{
	ModelViewer demo;
	demo.Run();
	return 0;
}
