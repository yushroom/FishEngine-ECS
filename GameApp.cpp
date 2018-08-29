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

static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	mainApp->Resize(width, height);
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

	glfwSetWindowSizeCallback(m_Window, glfwWindowSizeCallback);
	
	/*RenderSystem::GetInstance().Init2(m_WindowWidth, m_WindowHeight);*/
	m_Scene = new Scene();
	RenderSystem* rs = new RenderSystem();
	m_Scene->AddSystem(rs);

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
		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth), uint16_t(m_WindowHeight) );
		
		Update();
		
		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();


		/* Swap front and back buffers */
		//glfwSwapBuffers(window);

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
