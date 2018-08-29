#include "RenderSystem.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Renderable.hpp"
#include "GameApp.hpp"
#include "Graphics.hpp"

#include <bx/math.h>


SingletonRenderState::SingletonRenderState()
{
	m_UniformLightDir = bgfx::createUniform("lightDir", bgfx::UniformType::Vec4);
}


void RenderSystem::OnAdded()
{
	bgfx::Init init;
	init.type = bgfx::RendererType::Enum::OpenGL;
	init.resolution.width = 640;
	init.resolution.height = 480;
	init.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(init);
	bgfx::setDebug(BGFX_DEBUG_TEXT);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();

	state->m_State = 0
		| BGFX_STATE_WRITE_R
		| BGFX_STATE_WRITE_G
		| BGFX_STATE_WRITE_B
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CW
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

	auto renderState = m_Scene->GetSingletonComponent<SingletonRenderState>();

	Light* light = m_Scene->FindComponent<Light>();
	if (light != nullptr)
	{
		Vector3 d = Vector3::Normalize(light->direction);
		bgfx::setUniform(renderState->m_UniformLightDir, &d);
	}
	
	float view[16];
	bx::mtxLookAt(view, camera->eye, camera->at);
	
	float width = (float)GameApp::GetMainApp()->GetWidth();
	float height = (float)GameApp::GetMainApp()->GetHeight();
	float ratio = width / height;
	float proj[16];
	bx::mtxProj(proj, 60.0f, ratio, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(0, view, proj);
	
	
	m_Scene->ForEach<Renderable>([](GameObject* go, Renderable* rend)
	{
		float* mtx = go->GetTransform()->GetLocalToWorldMatrix();
		Graphics::DrawMesh(rend->mesh, mtx, rend->material);
	});
}

void RenderSystem::Resize(int width, int height)
{
	bgfx::reset(width, height);
}
