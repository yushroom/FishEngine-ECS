#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Texture.hpp>

union Params
{
	struct { float m_glossiness, m_reflectivity, m_exposure, m_bgType; };
	float floats[4];
};


class ModelViewer : public GameApp
{
public:
	void Start() override
	{

		//m_Shader = ShaderUtil::Compile("/Users/yushroom/program/FishEngine-ECS/shader/vs.bin", "/Users/yushroom/program/FishEngine-ECS/shader/fs.bin");
		{
			auto vs = "D:/program/FishEngine-ECS/shader/PBR_vs.bin";
			auto fs = "D:/program/FishEngine-ECS/shader/PBR_fs.bin";
			m_Shader = ShaderUtil::Compile(vs, fs);
		}

		{
			auto vs = "D:/program/FishEngine-ECS/shader/Skybox_vs.bin";
			auto fs = "D:/program/FishEngine-ECS/shader/Skybox_fs.bin";
			m_SkyboxShader = ShaderUtil::Compile(vs, fs);
		}

		{
			const char* path = R"(D:\program\bgfx\examples\runtime\textures\bolonga_lod.dds)";
			m_tex = TextureUtils::LoadTexture(path, BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | BGFX_TEXTURE_W_CLAMP);
			const char* filePath = R"(D:\program\bgfx\examples\runtime\textures\bolonga_irr.dds)";
			m_texIrr = TextureUtils::LoadTexture(filePath, BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | BGFX_TEXTURE_W_CLAMP);
		}

		s_texCube = bgfx::createUniform("s_texCube", bgfx::UniformType::Int1);
		s_texCubeIrr = bgfx::createUniform("s_texCubeIrr", bgfx::UniformType::Int1);
		u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);

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
			skyboxMat->m_Shader = m_SkyboxShader;
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
				mat->m_Shader = m_Shader;
				mat->pbrparams[0] = x * 0.1f;
				mat->pbrparams[1] = y * 0.1f;
				rend->material = mat;
			}
		}
		
		m_Scene->AddSystem(new FreeCameraSystem());
	}
	
	void Update() override
	{
		bgfx::setUniform(u_params, m_params.floats);
		bgfx::setTexture(0, s_texCube, m_tex);
		bgfx::setTexture(1, s_texCubeIrr, m_texIrr);
	}

private:
	Shader* m_Shader = nullptr;
	Shader* m_SkyboxShader = nullptr;
	bgfx::UniformHandle s_texCube;
	bgfx::UniformHandle s_texCubeIrr;
	bgfx::TextureHandle m_tex;
	bgfx::TextureHandle m_texIrr;
	bgfx::UniformHandle u_params;
	Params m_params;
};


int main(void)
{
	ModelViewer demo;
	demo.Run();
	return 0;
}
