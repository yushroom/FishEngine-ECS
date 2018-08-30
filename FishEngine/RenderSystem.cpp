#include "RenderSystem.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Renderable.hpp"
#include "GameApp.hpp"
#include "Graphics.hpp"
#include "Material.hpp"

#include <bx/math.h>


SingletonRenderState::SingletonRenderState()
{
	m_UniformLightDir = bgfx::createUniform("lightDir", bgfx::UniformType::Vec4);
	m_UniformBaseColor = bgfx::createUniform("BaseColor", bgfx::UniformType::Vec4);
	m_UniformPBRParams = bgfx::createUniform("PBRParams", bgfx::UniformType::Vec4);
	m_UniformCameraPos = bgfx::createUniform("CameraPos", bgfx::UniformType::Vec4);
}


void RenderSystem::OnAdded()
{
	bgfx::Init init;
	init.type = bgfx::RendererType::Enum::OpenGL;
	init.resolution.width = 640;
	init.resolution.height = 480;
	init.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(init);
	//bgfx::setDebug(BGFX_DEBUG_STATS);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();

	state->m_State = 0
		| BGFX_STATE_WRITE_R
		| BGFX_STATE_WRITE_G
		| BGFX_STATE_WRITE_B
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		;
}

void RenderSystem::Start()
{
	
}

void RenderSystem::Update()
{
	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(0);

	Camera* camera = m_Scene->FindComponent<Camera>();
	if (camera == nullptr)
		return;
	float cameraPos[4];
	float lookAt[3];
	{
		auto go = m_Scene->GetGameObjectByID(camera->entityID);
		auto& p = go->GetTransform()->position;
		cameraPos[0] = p.x;
		cameraPos[1] = p.y;
		cameraPos[2] = p.z;
		cameraPos[3] = 1.0f;

		lookAt[0] = camera->lookAt.x;
		lookAt[1] = camera->lookAt.y;
		lookAt[2] = camera->lookAt.z;
	}

	auto renderState = m_Scene->GetSingletonComponent<SingletonRenderState>();

	bgfx::setUniform(renderState->m_UniformCameraPos, cameraPos);

	Light* light = m_Scene->FindComponent<Light>();
	if (light != nullptr)
	{
		//Vector3 lightPos = { 0, 0, -1 };
		//Vector3 d = Vector3::Normalize(lightPos);
		Vector3 d = Vector3::Normalize(light->direction);
		bgfx::setUniform(renderState->m_UniformLightDir, &d);
	}

	float BaseColor[4] = { 1, 1, 1, 1 };
	//float PBRParams[] = { 0, 0.5, 0.5, 0 };
	bgfx::setUniform(renderState->m_UniformBaseColor, BaseColor);
	//bgfx::setUniform(renderState->m_UniformPBRParams, PBRParams);
	
	float view[16];
	bx::mtxLookAt(view, cameraPos, lookAt);
	
	float width = (float)GameApp::GetMainApp()->GetWidth();
	float height = (float)GameApp::GetMainApp()->GetHeight();
	float ratio = width / height;
	float proj[16];
	bx::mtxProj(proj, 60.0f, ratio, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(0, view, proj);
	
	
	m_Scene->ForEach<Renderable>([renderState](GameObject* go, Renderable* rend)
	{
		float* mtx = go->GetTransform()->GetLocalToWorldMatrix();
		bgfx::setUniform(renderState->m_UniformPBRParams, rend->material->pbrparams);
		Graphics::DrawMesh(rend->mesh, mtx, rend->material);
	});
}

void RenderSystem::Resize(int width, int height)
{
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}
