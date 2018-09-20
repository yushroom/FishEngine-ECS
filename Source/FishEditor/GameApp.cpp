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

#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Systems/DrawGizmosSystem.hpp>
#include <FishEditor/Systems/EditorSystem.hpp>
#include <FishEditor/Systems/SceneViewSystem.hpp>

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/pixelformat.h>

#include <imgui/imgui.h>

#	if BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#	elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#	endif
#include <GLFW/glfw3native.h>

#include <thread>


using namespace FishEditor;
using namespace FishEngine;

static void* glfwNativeWindowHandle(GLFWwindow* _window)
{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	return (void*)(uintptr_t)glfwGetX11Window(_window);
#	elif BX_PLATFORM_OSX
	return glfwGetCocoaWindow(_window);
#	elif BX_PLATFORM_WINDOWS
	return glfwGetWin32Window(_window);
#	endif // BX_PLATFORM_
}

static void glfwSetWindow(GLFWwindow* _window)
{
	bgfx::PlatformData pd;
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt      = glfwGetX11Display();
#	elif BX_PLATFORM_OSX
	pd.ndt      = NULL;
#	elif BX_PLATFORM_WINDOWS
	pd.ndt      = NULL;
#	endif // BX_PLATFORM_WINDOWS
	pd.nwh          = glfwNativeWindowHandle(_window);
	pd.context      = NULL;
	pd.backBuffer   = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
}

static GameApp* mainApp = nullptr;

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	mainApp->Resize(width, height);
}

inline KeyCode KKK(KeyCode key, int offset)
{
	return KeyCode(int(key) + offset);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN)
		return;

	KeyEvent e;
	if (action == GLFW_PRESS)
		e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
		e.action = KeyAction::Released;
	else if (action == GLFW_REPEAT)
		e.action = KeyAction::Held;
	else
		return;
	
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();

	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
		e.key = KeyCode(key);
	else if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
		e.key = KKK(KeyCode::A, key - GLFW_KEY_A);
	else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F15)
		e.key = KKK(KeyCode::F1, key - GLFW_KEY_F1);
	else if (key == GLFW_KEY_ESCAPE)
		e.key = KeyCode::Escape;
	else if (key == GLFW_KEY_LEFT_ALT)
		e.key = KeyCode::LeftAlt;
	else if (key == GLFW_KEY_RIGHT_ALT)
		e.key = KeyCode::RightAlt;
	else if (key == GLFW_KEY_LEFT_CONTROL)
		e.key = KeyCode::LeftControl;
	else if (key == GLFW_KEY_RIGHT_CONTROL)
		e.key = KeyCode::RightControl;
	else if (key == GLFW_KEY_LEFT_SUPER)
		e.key = KeyCode::LeftCommand;
	else if (key == GLFW_KEY_RIGHT_SUPER)
		e.key = KeyCode::RightCommand;
	else if (key == GLFW_KEY_LEFT)
		e.key = KeyCode::LeftArrow;
	else if (key == GLFW_KEY_RIGHT)
		e.key = KeyCode::RightArrow;
	else if (key == GLFW_KEY_UP)
		e.key = KeyCode::UpArrow;
	else if (key == GLFW_KEY_DOWN)
		e.key = KeyCode::DownArrow;
	s->PostKeyEvent(e);
	
	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	else if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;
	
	(void)mods; // Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	auto& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();
	KeyEvent e;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		e.key = KeyCode::MouseLeftButton;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		e.key = KeyCode::MouseRightButton;
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		e.key = KeyCode::MouseMiddleButton;

	if (action == GLFW_PRESS)
		e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
		e.action = KeyAction::Released;
	
	s->PostKeyEvent(e);
}

void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();
	s->UpdateAxis(Axis::MouseScrollWheel, (float)yoffset);
}

void glfw_window_iconify_callback(GLFWwindow* window, int iconified)
{
	mainApp->m_WindowMinimized = (iconified == GLFW_TRUE);
}

void glfw_window_focus_callback(GLFWwindow* window, int focused)
{
	mainApp->m_WindowMinimized = (focused != GLFW_TRUE);
}


void GameApp::Init()
{
	mainApp = this;

	/* Initialize the library */
	if (!glfwInit())
	{
		abort();
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	/* Create a windowed mode window and its OpenGL context */
	m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "FishEngine", NULL, NULL);
	if (!m_Window)
	{
		glfwTerminate();
		abort();
		return;
	}


	/* Make the window's context current */
	glfwMakeContextCurrent(m_Window);

	//bgfx::glfwSetWindow(window);
	glfwSetWindow(m_Window);

	glfwSetWindowSizeCallback(m_Window, glfw_window_size_callback);
	glfwSetKeyCallback(m_Window, glfw_key_callback);
	glfwSetMouseButtonCallback(m_Window, glfw_mouse_button_callback);
	glfwSetScrollCallback(m_Window, glfw_scroll_callback);
	glfwSetWindowIconifyCallback(m_Window, glfw_window_iconify_callback);
	glfwSetCharCallback(m_Window, glfw_char_callback);
	glfwSetWindowFocusCallback(m_Window, glfw_window_focus_callback);
	
	glfwSetWindowSizeLimits(m_Window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

	glfwSwapInterval(1);

	m_Scene = new Scene();
	Scene::s_Current = m_Scene;
	m_Scene->AddSystem<InputSystem>();
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
	auto es = m_EditorScene->AddSystem<EditorSystem>();
	es->m_GameScene = m_Scene;
	es->m_Priority = 999;
	this->OnWindowSizeChanged += [es](int w, int h) {
		es->m_WindowWidth = w;
		es->m_WindowHeight = h;
	};
	m_EditorScene->AddSystem<DrawGizmosSystem>();
	m_EditorScene->AddSystem<SelectionSystem>();
	auto svs = m_EditorScene->AddSystem<SceneViewSystem>();
	m_SceneViewSystem = svs;

	m_EditorSystem = es;


	Resize(m_WindowWidth, m_WindowHeight);
}


void GameApp::Run()
{
	Init();
	Start();

	m_Scene->Start();
	m_EditorScene->Start();
	
	while (!glfwWindowShouldClose(m_Window))
	{
		if (m_WindowMinimized)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
		}
		//glfwWaitEvents();
//		printf("============ frame begin ===========\n");

		auto si = m_EditorScene->GetSingletonComponent<SingletonInput>();
		if (si->IsButtonPressed(KeyCode::Escape))
			glfwSetWindowShouldClose(m_Window, 1);

		if (si->IsButtonPressed(KeyCode::F1))
		{
			bgfx::setDebug(BGFX_DEBUG_STATS);
		}
		else if (si->IsButtonReleased(KeyCode::F1))
		{
			bgfx::setDebug(BGFX_DEBUG_TEXT);
		}

		double cursor_x = 0, cursor_y = 0;
		glfwGetCursorPos(m_Window, &cursor_x, &cursor_y);
		cursor_x /= m_WindowWidth;
		cursor_y /= m_WindowHeight;
		m_EditorScene->GetSystem<InputSystem>()->SetMousePosition((float)cursor_x, 1.0f-(float)cursor_y);

		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Editor, 0, 0, m_WindowWidth, m_WindowHeight);
		
		m_EditorSystem->Draw();

		// Set view 0 default viewport.
//		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth*2), uint16_t(m_WindowHeight*2) );
		
//		Update();
		const int w = EditorScreen::width;
		const int h = EditorScreen::height;
		auto r = m_EditorSystem->m_SceneViewRect;
		//r = r + Vector4(8, 8, -16, -16);
		Screen::width = r.z;
		Screen::height = r.w;
		Vector2 old_mouse_position;
		{
			auto input1 = m_Scene->GetSingletonComponent<SingletonInput>();
			auto input2 = m_EditorScene->GetSingletonComponent<SingletonInput>();
			auto mp = input2->m_MousePosition;
			old_mouse_position = mp;
			mp.y = 1 - mp.y;
			mp = mp * Vector2(w, h) - Vector2(r.x, r.y);
			mp.x /= r.z;
			mp.y /= r.w;
			mp.y = 1 - mp.y;
			input1->m_MousePosition = mp;
			input2->m_MousePosition = mp;
			//printf("%f %f\n", mp.x, mp.y);

			memcpy(input1->m_Axis, input2->m_Axis, sizeof(float)*(int)Axis::AxisCount);
			memcpy(input1->m_KeyPressed, input2->m_KeyPressed, sizeof(KeyAction)*SingletonInput::ButtonCount);
		}

		m_EditorScene->Update();
		

		m_Scene->Update();
		//r = r * Vector4( w, h, w, h );
		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Scene, r.x, r.y, r.z, r.w);
		bgfx::setViewRect((bgfx::ViewId)RenderViewType::SceneGizmos, r.x, r.y, r.z, r.w);
		bgfx::setViewRect((bgfx::ViewId)RenderViewType::Picking, r.x, r.y, r.z, r.w);
		m_Scene->GetSystem<RenderSystem>()->Draw();
		m_SceneViewSystem->DrawGizmos();
		m_Scene->PostUpdate();
		//Screen::width = w;
		//Screen::height = h;
		m_EditorScene->PostUpdate();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		/* Swap front and back buffers */
		//glfwSwapBuffers(m_Window);

		auto input2 = m_EditorScene->GetSingletonComponent<SingletonInput>();
		input2->m_MousePosition = old_mouse_position;

		/* Poll for and process events */
		glfwPollEvents();
//		printf("============ frame end ===========\n");
	}

	/* Loop until the user closes the window */
	glfwTerminate();

}

void GameApp::Resize(int width, int height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;

	EditorScreen::width = width;
	EditorScreen::height = height;

	auto rs = m_Scene->GetSystem<RenderSystem>();
	rs->Resize(width, height);

	OnWindowSizeChanged(width, height);
}

GameApp* GameApp::GetMainApp()
{
	return mainApp;
}
