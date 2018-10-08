#include <FishEngine.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <FishEngine/Render/Window.h>

#include <FishEngine/Render/Application.h>

using namespace FishEngine;

inline std::string GetglTFSample(const std::string& name)
{
#ifdef __APPLE__
	return "/Users/yushroom/program/github/glTF-Sample-Models/2.0/"

#else
	return R"(D:\program\glTF-Sample-Models\2.0\)"
#endif
		+ name + "/glTF-Binary/" + name + ".glb";
}

#include <FishEngine/Render/CommandQueue.h>
#include <FishEngine/Render/Helpers.h>
#include <FishEngine/Render/d3dx12.h>
#include <FishEngine/Mesh.hpp>
#include <algorithm>
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

#include "../Source/FishEngine/Render/MeshImpl.hpp"
#include "../Source/FishEngine/Render/ShaderImpl.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void ResizeDepthBuffer(int width, int height, ComPtr<ID3D12Resource>& m_DepthBuffer, ComPtr<ID3D12DescriptorHeap>& m_DSVHeap)
{
	//if (m_ContentLoaded)
	{
		// Flush any GPU commands that might be referencing the depth buffer.
		Application::Get().Flush();

		width = std::max(1, width);
		height = std::max(1, height);

		auto device = Application::Get().GetDevice();

		// Resize screen dependent resources.
		// Create a depth buffer.
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
				1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&m_DepthBuffer)
		));

		// Update the depth-stencil view.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv,
			m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}


// Transition a resource
void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	Microsoft::WRL::ComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource.Get(),
		beforeState, afterState);

	commandList->ResourceBarrier(1, &barrier);
}


void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f)
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

ComPtr<IDXGISwapChain4> CreateSwapChain(
	int m_ClientWidth, int m_ClientHeight, UINT BufferCount, 
	bool m_IsTearingSupported, HWND m_hWnd, UINT & m_CurrentBackBufferIndex)
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
	swapChainDesc.Width = m_ClientWidth;
	swapChainDesc.Height = m_ClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = m_IsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
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

	m_CurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

	return dxgiSwapChain4;
}


// Update the render target views for the swapchain back buffers.
void UpdateRenderTargetViews(
	ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap, 
	UINT BufferCount, ComPtr<IDXGISwapChain4> m_dxgiSwapChain,
	ComPtr<ID3D12Resource> m_d3d12BackBuffers[3], UINT m_RTVDescriptorSize)
{
	auto device = Application::Get().GetDevice();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BufferCount; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		m_d3d12BackBuffers[i] = backBuffer;

		rtvHandle.Offset(m_RTVDescriptorSize);
	}
}


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	Application::Create(hInstance);

	Material::StaticInit();

	Scene* scene = new Scene();
	auto path = GetglTFSample("CesiumMan");
	GLTFLoadFlags flags;
	auto meshes = ModelUtil::LoadMeshesFromGLTF(path);

	XMMATRIX m_ModelMatrix;
	XMMATRIX m_ViewMatrix;
	XMMATRIX m_ProjectionMatrix;

	auto OnUpdate = [&m_ModelMatrix, &m_ViewMatrix, &m_ProjectionMatrix](UpdateEventArgs& e) {
		static uint64_t frameCount = 0;
		static double totalTime = 0.0;

		totalTime += e.ElapsedTime;
		frameCount++;

		if (totalTime > 1.0)
		{
			double fps = frameCount / totalTime;

			char buffer[512];
			sprintf_s(buffer, "FPS: %f\n", fps);
			OutputDebugStringA(buffer);

			frameCount = 0;
			totalTime = 0.0;
		}

		// Update the model matrix.
		float angle = static_cast<float>(e.TotalTime * 90.0);
		const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
		m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

		// Update the view matrix.
		const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
		const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
		m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

		// Update the projection matrix.
		//float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
		float aspectRatio = 1280.f / 800.f;
		float m_FoV = 45.0f;
		m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
	};


	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	auto m_Name = L"Editor";
	int m_Width = 1280;
	int m_Height = 800;
	bool m_vSync = false;
	//auto m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
	//m_pWindow->RegisterCallbacks();
	assert(glfwInit());
	auto window = glfwCreateWindow(m_Width, m_Height, "Hello", NULL, NULL);
	assert(window != nullptr);
	glfwMakeContextCurrent(window);
	HWND hWnd = glfwGetWin32Window(window);
	//m_pWindow->m_UpdateCallback = OnUpdate;
	//m_pWindow->Show();

	constexpr UINT BufferCount = 3;
	UINT m_CurrentBackBufferIndex = 0;
	auto m_dxgiSwapChain = CreateSwapChain(m_Width, m_Height, BufferCount, true, hWnd, m_CurrentBackBufferIndex);
	auto& app = Application::Get();
	bool m_IsTearingSupported = app.IsTearingSupported();
	auto m_d3d12RTVDescriptorHeap = app.CreateDescriptorHeap(BufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto m_RTVDescriptorSize = app.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];
	UpdateRenderTargetViews(m_d3d12RTVDescriptorHeap, BufferCount, m_dxgiSwapChain, m_d3d12BackBuffers, m_RTVDescriptorSize);


	auto device = Application::Get().GetDevice();
	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	ComPtr<ID3D12Resource> m_DepthBuffer;


	// Create the descriptor heap for the depth-stencil view.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

	ResizeDepthBuffer(m_Width, m_Height, m_DepthBuffer, m_DSVHeap);

	ComPtr<ID3D12PipelineState> m_PipelineState;
	D3D12_RECT m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	D3D12_VIEWPORT m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height));

	{
		auto device = Application::Get().GetDevice();
		//auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
		//auto commandList = commandQueue->GetCommandList();
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		} pipelineStateStream;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1;
		rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		auto& m_RootSignature = Material::ColorMaterial->GetShader()->m_Impl->m_RootSignature;
		auto& vertexShaderBlob = Material::ColorMaterial->GetShader()->m_Impl->m_VertexShaderBlob;
		auto& pixelShaderBlob = Material::ColorMaterial->GetShader()->m_Impl->m_PixelShaderBlob;

		pipelineStateStream.pRootSignature = m_RootSignature.Get();
		pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.RTVFormats = rtvFormats;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(PipelineStateStream), &pipelineStateStream
		};
		ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

		//auto fenceValue = commandQueue->ExecuteCommandList(commandList);
		//commandQueue->WaitForFenceValue(fenceValue);
	}
	   

	uint64_t m_FenceValues[Window::BufferCount] = {};
	Mesh* mesh = meshes[0];

	auto OnRender = [&](RenderEventArgs& e) {
		auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto commandList = commandQueue->GetCommandList();

		//UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
		UINT currentBackBufferIndex = m_CurrentBackBufferIndex;
		//auto backBuffer = m_pWindow->GetCurrentBackBuffer();
		auto backBuffer = m_d3d12BackBuffers[currentBackBufferIndex];
		//auto rtv = m_pWindow->GetCurrentRenderTargetView();
		auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentBackBufferIndex, m_RTVDescriptorSize);
		auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

		// Clear the render targets.
		{
			TransitionResource(commandList, backBuffer,
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

			ClearRTV(commandList, rtv, clearColor);
			ClearDepth(commandList, dsv);
		}

		auto material = Material::ColorMaterial;
		auto shader = material->GetShader()->m_Impl;
		//auto mesh = Mesh::Cube;

		commandList->SetPipelineState(m_PipelineState.Get());
		commandList->SetGraphicsRootSignature(shader->m_RootSignature.Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &mesh->m_Impl->m_VertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_Impl->m_IndexBufferView);

		commandList->RSSetViewports(1, &m_Viewport);
		commandList->RSSetScissorRects(1, &m_ScissorRect);

		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		// Update the MVP matrix
		XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
		mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
		commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

		commandList->DrawIndexedInstanced(mesh->m_Indices.size(), 1, 0, 0, 0);

		// Present
		{
			TransitionResource(commandList, backBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

			//currentBackBufferIndex = m_pWindow->Present();
			bool m_VSync = false;
			UINT syncInterval = m_VSync ? 1 : 0;
			UINT presentFlags = m_IsTearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			ThrowIfFailed(m_dxgiSwapChain->Present(syncInterval, presentFlags));
			m_CurrentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

			commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
		}
	};

	//m_pWindow->m_RenderCallback = OnRender;

	//MSG msg = { 0 };
	//while (msg.message != WM_QUIT)
	//{
	//	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}

	while (!glfwWindowShouldClose(window))
	{
		UpdateEventArgs e1(0.016f, glfwGetTime());
		OnUpdate(e1);
		RenderEventArgs e2(0.016f, glfwGetTime());
		OnRender(e2);
		glfwPollEvents();
	}
	glfwTerminate();

	// Flush any commands in the commands queues before quiting.
	Application::Get().Flush();

	//Application::Get().DestroyWindow(m_pWindow);
	//m_pWindow.reset();

	return 0;
}