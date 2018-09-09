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
#include <FishEngine/Assets.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>

#include <imgui/imgui.h>
#include <bx/bx.h>
#include <bx/rng.h>

#define RENDER_PASS_SHADING 0  // Default forward rendered geo with simple shading
#define RENDER_PASS_ID      1  // ID buffer for picking
#define RENDER_PASS_BLIT    2  // Blit GPU render target to CPU texture

union Params
{
	struct { float m_glossiness, m_reflectivity, m_exposure, m_bgType; };
	Vector4 floats;
	
	Params() : floats() {}
};


class PickingSystem : public ECS::ISystem
{
	static constexpr int ID_DIM = 4;	// Size of the ID buffer
public:
	void OnAdded() override
	{
		auto vs = FISHENGINE_ROOT "ThirdParty\\bgfx\\examples\\runtime\\shaders\\glsl\\vs_picking_shaded.bin";
		auto fs = FISHENGINE_ROOT "ThirdParty\\bgfx\\examples\\runtime\\shaders\\glsl\\fs_picking_id.bin";
		// Set up ID buffer, which has a color target and depth buffer
		m_idProgram = ShaderUtil::Compile(vs, fs);
		m_pickingRT = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::RGBA8, 0
			| BGFX_TEXTURE_RT
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
			| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
		);
		m_pickingRTDepth = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::D24S8, 0
			| BGFX_TEXTURE_RT
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
			| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
		);

		// CPU texture for blitting to and reading ID buffer so we can see what was clicked on.
		// Impossible to read directly from a render target, you *must* blit to a CPU texture
		// first. Algorithm Overview: Render on GPU -> Blit to CPU texture -> Read from CPU
		// texture.
		m_blitTex = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::RGBA8, 0
			| BGFX_TEXTURE_BLIT_DST
			| BGFX_TEXTURE_READ_BACK
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
			| BGFX_SAMPLER_MIP_POINT
			| BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
		);

		bgfx::TextureHandle rt[2] =
		{
			m_pickingRT,
			m_pickingRTDepth
		};
		m_pickingFB = bgfx::createFrameBuffer(BX_COUNTOF(rt), rt, true);

		m_idMaterial = new Material();
		m_idMaterial->SetShader(m_idProgram);

		// ID buffer clears to black, which represnts clicking on nothing (background)
		bgfx::setViewClear(RENDER_PASS_ID
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x0000ffff
			, 1.0f
			, 0
		);
	}

	void Update() override
	{
		auto input = m_Scene->GetSingletonComponent<SingletonInput>();
		Vector2 mousePos = input->GetMousePosition();
		float mx = mousePos.x * GameApp::GetMainApp()->GetWidth();
		float my = mousePos.y * GameApp::GetMainApp()->GetHeight();
		bgfx::setViewFrameBuffer(RENDER_PASS_ID, m_pickingFB);
		bgfx::setViewRect(RENDER_PASS_ID, mx, my, ID_DIM, ID_DIM);
		//auto& view = 
		std::map<Renderable*, Vector4> colors;
		bx::RngMwc mwc;  // Random number generator
		m_Scene->ForEach<Renderable>([&colors, &mwc, this](ECS::GameObject* go, Renderable* r) {
			Mesh* mesh = r->mesh;
			//Material* mat = r->material;
			uint32_t rr = mwc.gen() % 256;
			uint32_t gg = mwc.gen() % 256;
			uint32_t bb = mwc.gen() % 256;
			Vector4 color(rr / 255.0f, gg / 255.0f, bb / 255.0f, 1.0f);
			colors[r] = color;

			m_idMaterial->SetVector("u_id", color);
			Graphics::DrawMesh(mesh, go->GetTransform()->GetLocalToWorldMatrix(), m_idMaterial, RENDER_PASS_ID);
		});

		if (input->IsButtonPressed(KeyCode::MouseLeftButton))
		{
			// Blit and read
			bgfx::blit(RENDER_PASS_BLIT, m_blitTex, 0, 0, m_pickingRT);
			m_reading = bgfx::readTexture(m_blitTex, m_blitData);
			for (uint8_t p : m_blitData)
			{
				if (p != 0)
				{
					printf("here\n");
				}
			}
		}
	}

public:
	Shader* m_idProgram;
	Material* m_idMaterial;

	bgfx::UniformHandle u_id;
	bgfx::TextureHandle m_pickingRT;
	bgfx::TextureHandle m_pickingRTDepth;
	bgfx::TextureHandle m_blitTex;
	bgfx::FrameBufferHandle m_pickingFB;

	uint8_t m_blitData[ID_DIM*ID_DIM * 4]; // Read blit into this

	uint32_t m_reading;
	uint32_t m_currFrame;
};


class demo1 : public GameApp
{
public:
	void Start() override
	{
		{
			auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
			auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
			m_Shader = ShaderUtil::Compile(vs, fs);
		}

		{
			auto vs = FISHENGINE_ROOT "Shaders/runtime/Skybox_vs.bin";
			auto fs = FISHENGINE_ROOT "Shaders/runtime/Skybox_fs.bin";
			m_SkyboxShader = ShaderUtil::Compile(vs, fs);
		}

		{
			const char* path = FISHENGINE_ROOT "Assets/Textures/uffizi_cross_128_filtered.dds";
			m_tex_filtered = TextureUtils::LoadTexture(path, BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_W_CLAMP);
		}

		m_params.m_exposure = 0.0f;
		m_params.m_bgType = 1.0f;

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -15);
			m_Scene->GameObjectAddComponent<FreeCamera>(go);
			Skybox* skybox = m_Scene->GameObjectAddComponent<Skybox>(go);
			Material* skyboxMat = new Material();
			skyboxMat->SetShader(m_SkyboxShader);
			skyboxMat->SetVector("u_params", m_params.floats);
			skyboxMat->SetTexture("s_texCube", m_tex_filtered);
			skybox->m_skyboxMaterial = skyboxMat;
			go->m_Name = "Main Camera";
		}
		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(go);
			go->m_Name = "Directional Light";
		}
		

		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				auto go = m_Scene->CreateGameObject();
				Vector3 pos;
				pos.x = -7.5f + x * 1.5f;
				pos.y = -7.5f + y * 1.5f;
				go->GetTransform()->SetLocalPosition(pos);
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(go);
				rend->mesh = Mesh::Sphere;

				Material* mat = new Material();
				mat->SetShader(m_Shader);
				Vector4 pbrparams(x*0.1f, y*0.1f, 0, 0);
				mat->SetVector("BaseColor", Vector4::one);
				mat->SetVector("PBRParams", pbrparams);
				mat->SetTexture("AmbientCubemap", m_tex_filtered);
//				rend->material = mat;
				rend->m_Materials.push_back(mat);
			}
		}
		
		m_Scene->AddSystem(new FreeCameraSystem());
		//m_Scene->AddSystem(new PickingSystem());
	}
	
	void Update() override
	{
	}

private:
	Shader* m_Shader = nullptr;
	Shader* m_SkyboxShader = nullptr;
	bgfx::TextureHandle m_tex_filtered;
	Params m_params;
};


int main(void)
{
	demo1 demo;
	demo.Run();
	return 0;
}
