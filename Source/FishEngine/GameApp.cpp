#include "GameApp.hpp"

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/pixelformat.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Mesh.hpp"
#include "ECS.hpp"
#include "TransformSystem.hpp"
#include "RenderSystem.hpp"
#include "InputSystem.hpp"

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

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto s = mainApp->GetScene()->GetSystem<InputSystem>();
	KeyEvent e;
	if (action == GLFW_PRESS)
		e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
		e.action = KeyAction::Released;
	if (key == GLFW_KEY_F1)
		e.key = KeyCode::F1;
	if (key == GLFW_KEY_ESCAPE)
		e.key = KeyCode::ECS;
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

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
	m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Hello World", NULL, NULL);
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

	glfwSwapInterval(1);
	
	/*RenderSystem::GetInstance().Init2(m_WindowWidth, m_WindowHeight);*/
	m_Scene = new Scene();
	RenderSystem* rs = new RenderSystem();
	m_Scene->AddSystem(rs);

	m_Scene->AddSystem(new InputSystem());

	Mesh::StaticInit();

	m_Scene->AddSystem(new TransformSystem());

	Resize(m_WindowWidth, m_WindowHeight);
}


void GameApp::Run()
{
	Init();
	Start();
	
	while (!glfwWindowShouldClose(m_Window))
	{
		//glfwWaitEvents();

		auto si = m_Scene->GetSingletonComponent<SingletonInput>();
		if (si->IsButtonPressed(KeyCode::ECS))
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
		m_Scene->GetSystem<InputSystem>()->SetMousePosition(cursor_x, cursor_y);

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth), uint16_t(m_WindowHeight) );
		
		Update();
		m_Scene->Update();
		m_Scene->PostUpdate();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		/* Swap front and back buffers */
		//glfwSwapBuffers(m_Window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	/* Loop until the user closes the window */
	glfwTerminate();

}

void GameApp::Resize(int width, int height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	auto rs = m_Scene->GetSystem<RenderSystem>();
	rs->Resize(width, height);
}

GameApp* GameApp::GetMainApp()
{
	return mainApp;
}
