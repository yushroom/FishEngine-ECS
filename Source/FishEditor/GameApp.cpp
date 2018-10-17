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


#include <GLFW/glfw3.h>
//#include <bgfx/bgfx.h>
//#include <bgfx/platform.h>
//
//#include <bx/file.h>
//#include <bx/pixelformat.h>

#include <imgui.h>


#include <windows.h>
#include <FishEngine/Render/Application.h>

//#	if BX_PLATFORM_WINDOWS
//#define GLFW_EXPOSE_NATIVE_WIN32
//#	elif BX_PLATFORM_OSX
//#define GLFW_EXPOSE_NATIVE_COCOA
//#	endif
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <examples/imgui_impl_glfw.h>

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
//	bgfx::PlatformData pd;
//#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
//	pd.ndt      = glfwGetX11Display();
//#	elif BX_PLATFORM_OSX
//	pd.ndt      = NULL;
//#	elif BX_PLATFORM_WINDOWS
//	pd.ndt      = NULL;
//#	endif // BX_PLATFORM_WINDOWS
//	pd.nwh          = glfwNativeWindowHandle(_window);
//	pd.context      = NULL;
//	pd.backBuffer   = NULL;
//	pd.backBufferDS = NULL;
//	bgfx::setPlatformData(pd);
}

static GameApp* mainApp = nullptr;

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	int fbw = width;
	int fbh = height;
//	glfwGetFramebufferSize(window, &fbw, &fbh);
	mainApp->Resize(fbw, fbh);
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
	
	//ImGuiIO& io = ImGui::GetIO();
	//if (action == GLFW_PRESS)
	//	io.KeysDown[key] = true;
	//else if (action == GLFW_RELEASE)
	//	io.KeysDown[key] = false;
	//
	//(void)mods; // Modifiers are not reliable across systems
	//io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	//io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	//io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	//io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
	//auto& io = ImGui::GetIO();
	//if (c > 0 && c < 0x10000)
	//	io.AddInputCharacter((unsigned short)c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
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
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
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


#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_5.h>
using Microsoft::WRL::ComPtr;

#include <FishEngine/Render/Application.h>
#include <FishEngine/Render/Helpers.h>
#include <FishEngine/Render/CommandQueue.h>
#include <FishEngine/Render/d3dx12.h>
#include <FishEngine/Render/D3D12Context.hpp>
#include <FishEngine/Render/D3D12Utils.hpp>

#include <imgui.h>
#include <examples/imgui_impl_dx12.h>
//#include <examples/imgui_impl_win32.h>



ComPtr<IDXGISwapChain4> CreateSwapChain(
	int width, int height, UINT BufferCount,
	bool isTearingSupported, HWND m_hWnd, UINT & currentBackBufferIndex)
{
	Application& app = Application::Get();

	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = isTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	ID3D12CommandQueue* pCommandQueue = app.GetCommandQueue()->GetD3D12CommandQueue().Get();

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
		pCommandQueue,
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

	currentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

	return dxgiSwapChain4;
}


// Update the render target views for the swapchain back buffers.
void UpdateRenderTargetViews(
	ComPtr<ID3D12DescriptorHeap> d3d12RTVDescriptorHeap,
	UINT BufferCount, ComPtr<IDXGISwapChain4> dxgiSwapChain,
	ComPtr<ID3D12Resource> d3d12BackBuffers[3], UINT RTVDescriptorSize)
{
	auto device = Application::Get().GetDevice();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BufferCount; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		d3d12BackBuffers[i] = backBuffer;

		rtvHandle.Offset(RTVDescriptorSize);
	}
}


#include "../../Source/FishEngine/Render/D3D12WindowContext.hpp"

void D3D12WindowContext::Create(int width, int height, HWND hWnd)
{
	Application& app = Application::Get();
	m_IsTearingSupported = app.IsTearingSupported();
	m_dxgiSwapChain = CreateSwapChain(width, height, BufferCount, true, hWnd, m_CurrentBackBufferIndex);
	m_IsTearingSupported = app.IsTearingSupported();
	m_d3d12RTVDescriptorHeap = app.CreateDescriptorHeap(BufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_RTVDescriptorSize = app.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UpdateRenderTargetViews(m_d3d12RTVDescriptorHeap, BufferCount, m_dxgiSwapChain, m_d3d12BackBuffers, m_RTVDescriptorSize);
}


D3D12WindowContext context;


ID3D12GraphicsCommandList*   g_pd3dCommandList = NULL;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;


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
	
	glfwSetWindowSizeLimits(m_Window, m_WindowWidth, m_WindowHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

	glfwSwapInterval(1);


	HINSTANCE instance = GetModuleHandle(nullptr);
	Application::Create(instance);
	HWND hWnd = glfwGetWin32Window(m_Window);
	context.Create(m_WindowWidth, m_WindowHeight, hWnd);

	Material::StaticInit();


	m_Scene = new Scene();
	Scene::s_Current = m_Scene;
	m_Scene->AddSystem<InputSystem>();
	m_Scene->time = m_Scene->AddSingletonComponent<SingletonTime>();
	auto rs = m_Scene->AddSystem<RenderSystem>();
	rs->m_Priority = 1000;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//assert(ImGui_ImplWin32_Init(hWnd));
	ImGui_ImplGlfw_InitForOpenGL(m_Window, false);
	const auto& d3d12_context = rs->GetContext();
	//ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
	{
		auto g_pd3dDevice = Application::Get().GetDevice().Get();
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK);
	}
	ImGui_ImplDX12_Init(Application::Get().GetDevice().Get(), 3,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

	//{
	//	auto g_pd3dDevice = Application::Get().GetDevice().Get();
	//	ThrowIfFailed(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ))
	//}

	Texture::StaticInit();
	
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
	this->OnWindowSizeChanged += [es](int w, int h) {
		es->m_WindowWidth = w;
		es->m_WindowHeight = h;
	};
	m_EditorScene->AddSystem<DrawGizmosSystem>();
	m_EditorScene->AddSystem<SelectionSystem>();
	auto svs = m_EditorScene->AddSystem<SceneViewSystem>();
	m_SceneViewSystem = svs;

	m_EditorSystem = es;


//	Resize(m_WindowWidth, m_WindowHeight);
	glfw_window_size_callback(m_Window, m_WindowWidth, m_WindowHeight);
}


void GameApp::Run()
{
	Init();
	Start();

	m_Scene->Start();
	m_EditorScene->Start();

	double timeStamp = glfwGetTime();
	
	while (!glfwWindowShouldClose(m_Window))
	{
		if (m_WindowMinimized)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
		}
		//glfwWaitEvents();
//		printf("============ frame begin ===========\n");

		double now = glfwGetTime();
		m_Scene->time->deltaTime = float(now - timeStamp);
		timeStamp = now;
		m_EditorScene->time->deltaTime = m_Scene->time->deltaTime;
		//printf("%f\n", m_Scene->time->deltaTime);

		auto si = m_EditorScene->GetSingletonComponent<SingletonInput>();
		if (si->IsButtonPressed(KeyCode::Escape))
			glfwSetWindowShouldClose(m_Window, 1);

		if (si->IsButtonPressed(KeyCode::F1))
		{
			//bgfx::setDebug(BGFX_DEBUG_STATS);
		}
		else if (si->IsButtonReleased(KeyCode::F1))
		{
			//bgfx::setDebug(BGFX_DEBUG_TEXT);
		}



		double cursor_x = 0, cursor_y = 0;
		glfwGetCursorPos(m_Window, &cursor_x, &cursor_y);
		cursor_x /= m_WindowWidth;
		cursor_y /= m_WindowHeight;
		m_EditorScene->GetSystem<InputSystem>()->SetMousePosition((float)cursor_x, 1.0f-(float)cursor_y);

		//bgfx::setViewRect((bgfx::ViewId)RenderViewType::Editor, 0, 0, m_WindowWidth, m_WindowHeight);
		m_EditorSystem->Draw();

		// Set view 0 default viewport.
//		bgfx::setViewRect(0, 0, 0, uint16_t(m_WindowWidth*2), uint16_t(m_WindowHeight*2) );
		
//		Update();
		const int w = EditorScreen::width;
		const int h = EditorScreen::height;
		//auto r = m_EditorSystem->m_SceneViewRect;
		//r = r + Vector4(8, 8, -16, -16);
		Vector4 r(0, 0, w, h);
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
		//bgfx::setViewRect((bgfx::ViewId)RenderViewType::Scene, r.x, r.y, r.z, r.w);
		//bgfx::setViewRect((bgfx::ViewId)RenderViewType::SceneGizmos, r.x, r.y, r.z, r.w);
		//bgfx::setViewRect((bgfx::ViewId)RenderViewType::Picking, r.x, r.y, r.z, r.w);
		m_Scene->GetSystem<RenderSystem>()->Update();
		m_Scene->GetSystem<RenderSystem>()->Draw(context);
		//m_SceneViewSystem->DrawGizmos();
		m_Scene->PostUpdate();
		//Screen::width = w;
		//Screen::height = h;
		m_EditorScene->PostUpdate();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		//bgfx::frame();

		//ImGui_ImplDX12_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();

		//ImGui::ShowDemoWindow();

		//ImGui::Render();
		{
			//ImGui_ImplDX12_NewFrame();
			////ImGui_ImplWin32_NewFrame();
			//ImGui_ImplGlfw_NewFrame();
			//ImGui::NewFrame();

			//ImGui::ShowDemoWindow();

			ImGui::Render();
			auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
			auto commandList2 = commandQueue->GetCommandList();
			auto& context2 = m_Scene->GetSystem<RenderSystem>()->GetContext();
			//commandList2->RSSetViewports(1, &context2.m_Viewport);
			//commandList2->RSSetScissorRects(1, &context2.m_ScissorRect);
			commandList2->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
			auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(context.m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				context.m_CurrentBackBufferIndex, context.m_RTVDescriptorSize);
			auto dsv = context2.m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
			commandList2->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList2.Get());

			UINT currentBackBufferIndex = context.m_CurrentBackBufferIndex;
			//auto backBuffer = m_pWindow->GetCurrentBackBuffer();
			auto backBuffer = context.m_d3d12BackBuffers[currentBackBufferIndex];
			TransitionResource(commandList2, backBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			commandQueue->ExecuteCommandList(commandList2);
		}

		bool m_VSync = false;
		UINT syncInterval = m_VSync ? 1 : 0;
		UINT presentFlags = context.m_IsTearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(context.m_dxgiSwapChain->Present(syncInterval, presentFlags));
		context.m_CurrentBackBufferIndex = context.m_dxgiSwapChain->GetCurrentBackBufferIndex();


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
