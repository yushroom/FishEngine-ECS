#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "GameApp.hpp"
#include "IMGUI.hpp"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_glfw.h>

#include <GameCore.h>
#include <SystemTime.h>
#include <PostEffects.h>
#include <CommandContext.h>
#include <BufferManager.h>

#include "Graphics.hpp"

namespace GameCore
{
	//void InitializeApplication(IGameApp& game);
	//bool UpdateApplication(IGameApp& game);
	//void TerminateApplication(IGameApp& game);
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

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static uint16_t g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};


//void Initialize(IGameApp& game)
void Initialize()
{
	Graphics::Initialize();
	SystemTime::Initialize();
	//GameInput::Initialize();
	EngineTuning::Initialize();

	//game.Startup();

	FishEngine::VertexDecl decl;
	decl.size = sizeof(VertexPosColor);
	auto vb = FishEngine::CreateVertexBuffer(FishEngine::Memory{ g_Vertices, sizeof(g_Vertices) }, decl);
	auto ib = FishEngine::CreateIndexBuffer(FishEngine::Memory{ g_Indicies, sizeof(g_Indicies) }, FishEngine::MeshIndexType::UInt16);
}

extern UserDescriptorHeap g_imguiSrvDescHeap;

// bool Update(IGameApp& game)
void Update()
{
	EngineProfiling::Update();

	float DeltaTime = Graphics::GetFrameTime();

	//GameInput::Update(DeltaTime);
	EngineTuning::Update(DeltaTime);

	//game.Update(DeltaTime);
	//game.RenderScene();
#if 1
	auto& gfxContext = GraphicsContext::Begin(L"IMGUI");
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Begin("Engine Tuning");
	VariableGroup::sm_RootGroup.OnImGUI("Engine Tuning");
	ImGui::End();

	ImGui::Render();

	gfxContext.TransitionResource(Graphics::g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.ClearColor(Graphics::g_SceneColorBuffer);
	
	gfxContext.SetRenderTarget(Graphics::g_SceneColorBuffer.GetRTV());
	auto commandList = gfxContext.GetCommandList();
	//commandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
	gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, g_imguiSrvDescHeap.GetHeapPointer());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	gfxContext.Finish();
#endif

	//PostEffects::Render();

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

#if 0
	auto g_OverlayBuffer = Graphics::g_OverlayBuffer;
	GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
	UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	UiContext.ClearColor(g_OverlayBuffer);
	UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
	UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
	//game.RenderUI(UiContext);

	EngineTuning::Display(UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

	UiContext.Finish();
#endif

	Graphics::Present();

	//return !game.IsDone();
}

GLFWwindow* g_gWindow = nullptr;

namespace FishEngine
{
	class WinGameApp : public GameApp
	{
	public:
		virtual void Start() override;
		virtual void Update() override;
		virtual void Clean() override;
	};

	void WinGameApp::Start()
	{
		g_gWindow = m_Window;

		Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
		ASSERT_SUCCEEDED(InitializeWinRT);

		GameCore::g_hWnd = glfwGetWin32Window(m_Window);

		Initialize();
		InitImgui();
	}

	void WinGameApp::Update()
	{
		::Update();
	}

	void WinGameApp::Clean()
	{
		Graphics::Terminate();
		//TerminateApplication(app);
		Graphics::Shutdown();
	}

}

int main()
{
	FishEngine::WinGameApp app;
	app.Run();
	return 0;
}