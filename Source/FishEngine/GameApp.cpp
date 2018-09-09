#include <FishEngine/GameApp.hpp>
#include "FishEngine/Mesh.hpp"
//#include "ECS.hpp"
#include "FishEngine/Systems/TransformSystem.hpp"
#include "FishEngine/Systems/RenderSystem.hpp"
#include "FishEngine/Systems/InputSystem.hpp"
#include "FishEngine/Screen.hpp"
#include <FishEngine/Material.hpp>

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/pixelformat.h>

#	if BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#	elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#	endif
#include <GLFW/glfw3native.h>


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
	
	auto s = mainApp->GetScene()->GetSystem<InputSystem>();

	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
		e.key = KeyCode(key);
	else if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
		e.key = KeyCode(key);
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
	s->PostKeyEvent(e);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto s = mainApp->GetScene()->GetSystem<InputSystem>();
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
	auto s = mainApp->GetScene()->GetSystem<InputSystem>();
	s->UpdateAxis(Axis::MouseScrollWheel, yoffset);
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

	glfwSwapInterval(1);
	
	/*RenderSystem::GetInstance().Init2(m_WindowWidth, m_WindowHeight);*/
	m_Scene = new ECS::Scene();
	RenderSystem* rs = new RenderSystem();
	rs->m_Priority = 1000;
	m_Scene->AddSystem(rs);

	m_Scene->AddSystem(new InputSystem());

	Mesh::StaticInit();
	Material::StaticInit();
	
	auto s = new TransformSystem();
	s->m_Priority = 999;
	m_Scene->AddSystem(s);

	Resize(m_WindowWidth, m_WindowHeight);
}


void GameApp::Run()
{
	Init();
	Start();

	m_Scene->Start();
	
	while (!glfwWindowShouldClose(m_Window))
	{
		//glfwWaitEvents();
//		printf("============ frame begin ===========\n");

		auto si = m_Scene->GetSingletonComponent<SingletonInput>();
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
		m_Scene->GetSystem<InputSystem>()->SetMousePosition(cursor_x, cursor_y);

		// Set view 0 default viewport.
//		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth*2), uint16_t(m_WindowHeight*2) );
		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth), uint16_t(m_WindowHeight) );
		
		Update();
		m_Scene->Update();

		m_Scene->GetSystem<RenderSystem>()->Draw();

		m_Scene->PostUpdate();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		/* Swap front and back buffers */
		//glfwSwapBuffers(m_Window);

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

	Screen::width = width;
	Screen::height = height;

	auto rs = m_Scene->GetSystem<RenderSystem>();
	rs->Resize(width, height);
}

GameApp* GameApp::GetMainApp()
{
	return mainApp;
}
