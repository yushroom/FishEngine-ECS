#include <FishEditor/GameApp.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Texture.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Render/RenderViewType.hpp>
#include <FishEngine/Systems/TransformSystem.hpp>
#include <FishEngine/Systems/RenderSystem.hpp>
#include <FishEngine/Systems/InputSystem.hpp>
#include <FishEngine/Components/SingletonTime.hpp>

#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Systems/DrawGizmosSystem.hpp>
#include <FishEditor/Systems/EditorSystem.hpp>
#include <FishEditor/Systems/SceneViewSystem.hpp>
#include <FishEngine/GraphicsAPI.hpp>

#include <FishEditor/Systems/FreeCameraSystem.hpp>

#include <imgui.h>

#include <thread>

using namespace FishEditor;
using namespace FishEngine;


GameApp* mainApp = nullptr;


void GameApp::Init()
{
	mainApp = this;

	Screen::width = 800*2;
	Screen::height = 600*2;
	
	FishEngine::InitGraphicsAPI();
	

	m_Scene = new Scene();
	Scene::s_Current = m_Scene;
	m_Scene->AddSystem<InputSystem>();
	m_Scene->time = m_Scene->AddSingletonComponent<SingletonTime>();
	auto rs = m_Scene->AddSystem<RenderSystem>();
	rs->m_Priority = 1000;

	Texture::StaticInit();
	Material::StaticInit();
	Mesh::StaticInit();
	Gizmos::StaticInit();
	
	{
		auto s = m_Scene->AddSystem<TransformSystem>();
		s->m_Priority = 998;
	}

	m_EditorScene = new Scene();
	m_EditorScene->AddSystem<InputSystem>();
	m_EditorScene->time = m_EditorScene->AddSingletonComponent<SingletonTime>();
	auto es = m_EditorScene->AddSystem<EditorSystem>();
	es->m_GameScene = m_Scene;
	es->m_Priority = 999;
//	this->OnWindowSizeChanged += [es](int w, int h) {
//		es->m_WindowWidth = w;
//		es->m_WindowHeight = h;
//	};
	m_EditorScene->AddSystem<DrawGizmosSystem>();
	m_EditorScene->AddSystem<SelectionSystem>();
	auto svs = m_EditorScene->AddSystem<SceneViewSystem>();
	m_SceneViewSystem = svs;

	m_EditorSystem = es;

}


void GameApp::Start()
{
	m_Scene->Start();
	m_EditorScene->Start();
}


void GameApp::Update()
{
//		if (m_WindowMinimized)
//		{
//			using namespace std::chrono_literals;
//			std::this_thread::sleep_for(200ms);
//		}
//glfwWaitEvents();
//		printf("============ frame begin ===========\n");
	

	
	
//		frameCount ++;
//		if (frameCount == 100)
//		{
//			double now = glfwGetTime();
//			float time = now - timeStamp;
//			float fps = 100 / time;
//			printf("[FPS]%f %f\n", time, fps);
//			timeStamp = now;
//			frameCount = 0;
//		}
	
	auto si = m_EditorScene->input;
//	if (si->IsButtonPressed(KeyCode::Escape))
//		glfwSetWindowShouldClose(m_Window, 1);
	
	static bool show_editor = true;
	if (si->IsButtonPressed(KeyCode::F1))
	{
//			bgfx::setDebug(BGFX_DEBUG_STATS);
		show_editor = !show_editor;
	}
	
	FishEngine::BeginFrame();
	FishEngine::ClearColorDepthBuffer();
	
//	double cursor_x = 0, cursor_y = 0;
//	glfwGetCursorPos(m_Window, &cursor_x, &cursor_y);
//	cursor_x /= m_WindowWidth;
//	cursor_y /= m_WindowHeight;
//	m_EditorScene->GetSystem<InputSystem>()->SetMousePosition((float)cursor_x, 1.0f-(float)cursor_y);
	
//		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Editor, 0, 0, m_WindowWidth, m_WindowHeight);

//		m_EditorSystem->Draw();

// Set view 0 default viewport.
//		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth*2), uint16_t(m_WindowHeight*2) );

//		Update();
	const int w = EditorScreen::width;
	const int h = EditorScreen::height;
//		auto r = m_EditorSystem->m_SceneViewRect;
	auto r = Vector4(0, 0, w, h);		// TODO
//r = r + Vector4(8, 8, -16, -16);
//		Screen::width = r.z;
//		Screen::height = r.w;
	Vector2 old_mouse_position;
	{
		auto input = m_Scene->input;
		auto editorInput = m_EditorScene->input;
		auto mp = editorInput->m_MousePosition;
		old_mouse_position = mp;
		mp.y = 1 - mp.y;
		mp = mp * Vector2(w, h) - Vector2(r.x, r.y);
		mp.x /= r.z;
		mp.y /= r.w;
		mp.y = 1 - mp.y;
		input->m_MousePosition = mp;
		editorInput->m_MousePosition = mp;
		//printf("%f %f\n", mp.x, mp.y);
		
		auto& io = ImGui::GetIO();
		if (!io.WantCaptureMouse)
		memcpy(input->m_Axis, editorInput->m_Axis, sizeof(float)*(int)Axis::AxisCount);
		if (!io.WantCaptureKeyboard)
		memcpy(input->m_KeyPressed, editorInput->m_KeyPressed, sizeof(KeyAction)*SingletonInput::ButtonCount);
		
		if (io.WantCaptureMouse)
		m_EditorScene->GetSystem<FreeCameraSystem>()->m_Enabled = false;
	}
	
	
// TODO
//		std::swap(m_Scene->input, m_EditorScene->input);
//		auto einput = m_EditorScene->input;
//		m_EditorScene->input = m_Scene->input;
	m_EditorScene->Update();
	m_EditorScene->GetSystem<FreeCameraSystem>()->m_Enabled = true;
//		std::swap(m_Scene->input, m_EditorScene->input);
//		m_EditorScene->input = einput;
	
	m_Scene->Update();
//r = r * Vector4( w, h, w, h );
//		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Scene, r.x, r.y, r.z, r.w);
//		bgfx::setViewRect((bgfx::ViewId)RenderViewType::SceneGizmos, r.x, r.y, r.z, r.w);
//		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Picking, r.x, r.y, r.z, r.w);
	m_Scene->GetSystem<RenderSystem>()->Draw();
	m_SceneViewSystem->DrawGizmos();
	
	
	FishEngine::EndPass();
	if (show_editor)
		m_EditorSystem->Draw();
	
	m_Scene->PostUpdate();
	//Screen::width = w;
	//Screen::height = h;
	m_EditorScene->PostUpdate();
	
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	//		bgfx::frame();
	FishEngine::EndFrame();
	
	/* Swap front and back buffers */
	//glfwSwapBuffers(m_Window);
	
	auto input2 = m_EditorScene->input;
	input2->m_MousePosition = old_mouse_position;
}


void GameApp::Run()
{
	Init();
	Start();
	
	while (true)
	{
		m_Scene->time->deltaTime = 0.01666f;
		m_Scene->time->deltaTime = m_Scene->time->deltaTime;
		
		Update();
		
		if (m_GameOver)
			break;
	}
	
	CleanUp();
}


//void GameApp::Run()
//{
//	Init();
//	Start();
//
//	m_Scene->Start();
//	m_EditorScene->Start();
//
//	int frameCount = 0;
//	double timeStamp = glfwGetTime();
//
//	while (!glfwWindowShouldClose(m_Window))
//	{
//	}
//
//	/* Loop until the user closes the window */
//	glfwTerminate();
//}

void GameApp::Resize(int width, int height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;

	EditorScreen::width = width;
	EditorScreen::height = height;
	
	Screen::width = width;
	Screen::height = height;

	auto rs = m_Scene->GetSystem<RenderSystem>();
	rs->Resize(width, height);

	OnWindowSizeChanged(width, height);
}

GameApp* GameApp::GetMainApp()
{
	return mainApp;
}
