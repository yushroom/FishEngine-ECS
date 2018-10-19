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

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	if (width == 0)
		width = 1;
	if (height == 0)
		height = 1;
	printf("resize: w=%d h=%d\n", width, height);
	Graphics::Resize(width, height);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
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

	ModelViewer app;
	//GameCore::InitializeApplication(app);
	Initialize(app);


	while (!glfwWindowShouldClose(g_gWindow))
	{
		//UpdateApplication(app);
		Update(app);
		glfwPollEvents();
	}

	Graphics::Terminate();
	TerminateApplication(app);
	Graphics::Shutdown();

	return 0;
}