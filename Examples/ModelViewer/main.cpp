#include <GLFW/glfw3.h>
#include <cstdlib>
#include "ModelViewer.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>

#include <SystemTime.h>
#include <PostEffects.h>
#include <CommandContext.h>
#include <BufferManager.h>

#include <FishEngine/Screen.hpp>



namespace GameCore
{
	void InitializeApplication(IGameApp& game);
	bool UpdateApplication(IGameApp& game);
	void TerminateApplication(IGameApp& game);
}

void Initialize(IGameApp& game)
{
	Graphics::Initialize();
	SystemTime::Initialize();
	//GameInput::Initialize();
	EngineTuning::Initialize();

	game.Startup();
}

bool Update(IGameApp& game)
{
	EngineProfiling::Update();

	float DeltaTime = Graphics::GetFrameTime();

	//GameInput::Update(DeltaTime);
	EngineTuning::Update(DeltaTime);

	game.Update(DeltaTime);
	game.RenderScene();

	PostEffects::Render();

#if 0
	if (TestGenerateMips)
	{
		GraphicsContext& MipsContext = GraphicsContext::Begin();

		// Exclude from timings this copy necessary to setup the test
		MipsContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		MipsContext.TransitionResource(g_GenMipsBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		MipsContext.CopySubresource(g_GenMipsBuffer, 0, g_SceneColorBuffer, 0);

		EngineProfiling::BeginBlock(L"GenerateMipMaps()", &MipsContext);
		g_GenMipsBuffer.GenerateMipMaps(MipsContext);
		EngineProfiling::EndBlock(&MipsContext);

		MipsContext.Finish();
	}
#endif
	GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
	UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	UiContext.ClearColor(g_OverlayBuffer);
	UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
	UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
	game.RenderUI(UiContext);

	EngineTuning::Display(UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

	UiContext.Finish();


	Graphics::Present();

	return !game.IsDone();
}


namespace GameCore
{
	extern HWND g_hWnd;
}

namespace Graphics
{
	extern uint32_t g_DisplayWidth;
	extern uint32_t g_DisplayHeight;
}

ModelViewer* mainApp = nullptr;
using namespace FishEngine;
#include <FishEngine/Systems/InputSystem.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/Components/SingletonTime.hpp>

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	if (width == 0)
		width = 1;
	if (height == 0)
		height = 1;
	printf("resize: w=%d h=%d\n", width, height);
	Graphics::Resize(width, height);
}

inline KeyCode KKK(KeyCode key, int offset)
{
	return KeyCode(int(key) + offset);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (key == GLFW_KEY_UNKNOWN)
		return;

	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	KeyEvent e;
	if (action == GLFW_PRESS)
		e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
		e.action = KeyAction::Released;
	else if (action == GLFW_REPEAT)
		e.action = KeyAction::Held;
	else
		return;

	auto s = mainApp->m_Scene->GetSystem<InputSystem>();

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
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	auto s = mainApp->m_Scene->GetSystem<InputSystem>();
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
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}


GLFWwindow* g_gWindow = nullptr;

int main()
{
	Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
	ASSERT_SUCCEEDED(InitializeWinRT);

	if (!glfwInit())
	{
		abort();
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	/* Create a windowed mode window and its OpenGL context */
	g_gWindow = glfwCreateWindow(g_DisplayWidth, g_DisplayHeight, "FishEngine", NULL, NULL);
	if (!g_gWindow)
	{
		glfwTerminate();
		abort();
		return 1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(g_gWindow);

	glfwSetWindowSizeCallback(g_gWindow, glfw_window_size_callback);
	glfwSetKeyCallback(g_gWindow, glfw_key_callback);
	glfwSetMouseButtonCallback(g_gWindow, glfw_mouse_button_callback);
	glfwSetScrollCallback(g_gWindow, glfw_scroll_callback);
	//glfwSetWindowIconifyCallback(g_gWindow, glfw_window_iconify_callback);
	glfwSetCharCallback(g_gWindow, glfw_char_callback);
	//glfwSetWindowFocusCallback(g_gWindow, glfw_window_focus_callback);


	g_hWnd = glfwGetWin32Window(g_gWindow);

	//g_DisplayWidth = 800;
	//g_DisplayHeight = 600;
	FishEngine::Screen::width = g_DisplayWidth;
	FishEngine::Screen::height = g_DisplayHeight;

	ModelViewer app;
	mainApp = &app;
	//GameCore::InitializeApplication(app);
	Initialize(app);


	double timeStamp = glfwGetTime();

	while (!glfwWindowShouldClose(g_gWindow))
	{
		double now = glfwGetTime();
		app.m_Scene->time->deltaTime = float(now - timeStamp);

		//UpdateApplication(app);
		Update(app);
		glfwPollEvents();
	}

	Graphics::Terminate();
	TerminateApplication(app);
	Graphics::Shutdown();

	return 0;
}