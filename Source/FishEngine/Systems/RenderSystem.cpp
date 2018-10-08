#include <FishEngine/Systems/RenderSystem.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/SingletonRenderState.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/ECS/Scene.hpp>
//#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/bgfxHelper.hpp>
#include <FishEngine/Render/RenderViewType.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Render/CameraFrustumCulling.hpp>

using namespace FishEngine;

#if 0

void RenderSystem::OnAdded()
{
	bgfx::Init init;
#if FISHENGINE_PLATFORM_APPLE
	init.type = bgfx::RendererType::Enum::Metal;
#else
	init.type = bgfx::RendererType::Enum::Direct3D12;
#endif
	init.resolution.width = 800;
	init.resolution.height = 600;
	init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8;
	init.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(init);
//	bgfx::setDebug(BGFX_DEBUG_STATS);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x303030ff, 1.0f, 0);

	auto state = m_Scene->AddSingletonComponent<SingletonRenderState>();

	state->m_State = 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
//		| BGFX_STATE_MSAA
		;
	
	//assert(bgfx::getCaps()->supported & BGFX_CAPS_COMPUTE);
}

void RenderSystem::Start()
{
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Scene,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::UI,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Editor,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::SceneGizmos,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
//	bgfx::setViewClear((bgfx::ViewId)RenderViewType::Picking,
//					   BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
//					   0x000000ff, 1.0f, 0);
}

#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Screen.hpp>

Matrix4x4 u_jointMatrix[256];

void RenderSystem::Draw()
{
//	bgfx::touch((bgfx::ViewId)RenderViewType::SceneGizmos);
	Camera* camera = Camera::GetEditorCamera();
	if (camera == nullptr)
		return;
	Vector4 cameraPos(camera->GetTransform()->GetPosition(), 1.0f);

	auto renderState = m_Scene->GetSingletonComponent<SingletonRenderState>();

	bgfx::setUniform(renderState->m_UniformCameraPos, cameraPos.data());

	Light* light = m_Scene->FindComponent<Light>();
	if (light != nullptr)
	{
		Vector3 lightDir = -light->m_GameObject->GetTransform()->GetForward();
		Vector3 d = Vector3::Normalize(lightDir);
		bgfx::setUniform(renderState->m_UniformLightDir, &d);
	}
	
	
	float width = (float)Screen::width;
	float height = (float)Screen::height;
	float aspectRatio = width / height;
	Matrix4x4 viewMat = camera->GetWorldToCameraMatrix();
	Matrix4x4 projMat = camera->GetProjectionMatrix();
	Matrix4x4 viewT = viewMat.transpose();
	Matrix4x4 projT = projMat.transpose();
	bgfx::setViewTransform((bgfx::ViewId)RenderViewType::Scene, viewT.data(), projT.data());
	bgfx::setViewTransform((bgfx::ViewId)RenderViewType::SceneGizmos, viewT.data(), projT.data());
	
	// draw skybox first
	auto old_state = renderState->m_State;
	renderState->m_State = BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS;
	m_Scene->ForEach<Skybox>([cameraPos](GameObject* go, Skybox* skybox)
	{
		auto mat = Matrix4x4::TRS(Vector3(cameraPos[0], cameraPos[1], cameraPos[2]), Quaternion::identity, Vector3::one*100);
		Graphics::DrawMesh(Mesh::Sphere, mat, skybox->m_SkyboxMaterial);
	});
	renderState->m_State = old_state;

#if 1
	// CPU skinning
	
	m_Scene->ForEach<Renderable>([](GameObject* go, Renderable* r)
	{
		if (r == nullptr || !r->m_Enabled)
		{
			return;
		}
		auto mesh = r->m_Mesh;
		if (mesh != nullptr && mesh->IsSkinned())
		{
			auto skin = r->m_Skin;
			const auto worldToObject = r->GetTransform()->GetWorldToLocalMatrix();
			for (int i = 0; i < skin->joints.size(); ++i)
			{
				auto bone = skin->joints[i]->GetTransform();
				auto& bindpose = skin->inverseBindMatrices[i];
				u_jointMatrix[i] = worldToObject * bone->GetLocalToWorldMatrix() * bindpose;
			}
			
			
			if (mesh->m_DynamicVertices.size() != mesh->m_Vertices.size())
			{
//				mesh->m_DynamicVertices.resize(mesh->m_Vertices.size());
//				memcpy(mesh->m_DynamicVertices.data(), mesh->m_Vertices.data(), sizeof(mesh->m_Vertices));
				mesh->m_DynamicVertices = mesh->m_Vertices;
			}
			for (int i = 0; i < mesh->m_Vertices.size(); ++i)
			{
				const Vector4& a_weight = mesh->weights[i];
				const auto& a_joint = mesh->joints[i];
				Matrix4x4 skinMatrix = u_jointMatrix[a_joint.x] * a_weight.x;
				if (a_weight.y > 0)
					skinMatrix += u_jointMatrix[a_joint.y] * a_weight.y;
				if (a_weight.z > 0)
					skinMatrix += u_jointMatrix[a_joint.z] * a_weight.z;
				if (a_weight.w > 0)
					skinMatrix += u_jointMatrix[a_joint.w] * a_weight.w;
				auto& dv = mesh->m_DynamicVertices[i];
				auto& v = mesh->m_Vertices[i];
				dv.position = skinMatrix.MultiplyPoint(v.position);
				dv.normal = skinMatrix.MultiplyVector(v.normal);
			}
			
			
			auto mem = bgfxHelper::MakeRef(mesh->m_DynamicVertices);
			if (!bgfx::isValid(mesh->m_DynamicVertexBuffer))
			{
				mesh->m_DynamicVertexBuffer = bgfx::createDynamicVertexBuffer(mem, PUNTVertex::ms_decl);
			}
			else
			{
				bgfx::update(mesh->m_DynamicVertexBuffer, 0, mem);
			}
		}
	});
#endif
	
	// test frustum culling
//	auto gameCamera = Camera::GetMainCamera();
	CameraFrustumCulling culling(camera, aspectRatio);


#if 1
	m_Scene->ForEach<Renderable>([&culling, renderState](GameObject* go, Renderable* rend)
	{
		if (rend == nullptr || !rend->m_Enabled || rend->m_Mesh == nullptr)
			return;

		auto& modelMat = go->GetTransform()->GetLocalToWorldMatrix();


		bool visiable = true;
		if (renderState->m_EnableFrustumCulling)
		{
			if (!rend->m_Mesh->IsSkinned())
				visiable = culling.Visiable(rend->m_Mesh, modelMat);
		}

#if 0
		if (visiable)
			Gizmos::color = Vector4(0, 1, 0, 1);
		else
			Gizmos::color = Vector4(0, 0, 1, 1);
		Gizmos::matrix = modelMat;
		Gizmos::DrawBounds(rend->mesh->bounds);
#endif
		
		if (!visiable)
			return;
		
		if (rend->m_Materials.empty())
		{
			// render with error material
			Graphics::DrawMesh(rend->m_Mesh, modelMat, Material::ErrorMaterial);
		}
		else
		{
			if (rend->m_Materials.size() != rend->m_Mesh->m_SubMeshCount)
			{
				abort();	// mismatch
				Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[0]);
			}
			else
			{
				if (rend->m_Materials.size() == 1)
				{
					Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[0]);
				}
				else
				{
					for (int i = 0; i < rend->m_Materials.size(); ++i)
					{
						Graphics::DrawMesh(rend->m_Mesh, modelMat, rend->m_Materials[i], 0, i);
					}
				}
			}
		}
	});
#endif
	
	Gizmos::__Draw();
}


void RenderSystem::Resize(int width, int height)
{
//	bgfx::reset(width*2, height*2, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}

#endif


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_5.h>
using Microsoft::WRL::ComPtr;
#include <DirectXMath.h>
using namespace DirectX;

#include <FishEngine/Render/Application.h>
#include <FishEngine/Render/Helpers.h>
#include <FishEngine/Render/CommandQueue.h>
#include <FishEngine/Render/d3dx12.h>

#include <FishEngine/Shader.hpp>
#include "../Render/ShaderImpl.hpp"
#include "../Render/MeshImpl.hpp"
#include "../Render/D3D12WindowContext.hpp"
//#include <FishEditor/GameApp.hpp>

#include <algorithm>
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

void ResizeDepthBuffer(int width, int height, ComPtr<ID3D12Resource>& depthBuffer, ComPtr<ID3D12DescriptorHeap>& DSVHeap)
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
			IID_PPV_ARGS(&depthBuffer)
		));

		// Update the depth-stencil view.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(depthBuffer.Get(), &dsv,
			DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

struct D3D12Context
{
	//bool m_VSync = false;
	//ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
	//ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap;
	//ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];
	//UINT m_RTVDescriptorSize;
	//UINT m_CurrentBackBufferIndex;
	//bool m_IsTearingSupported;

	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	ComPtr<ID3D12Resource> m_DepthBuffer;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	uint64_t m_FenceValues[3] = {};
	
	D3D12_RECT m_ScissorRect;
	D3D12_VIEWPORT m_Viewport;

	XMMATRIX m_ModelMatrix;
	XMMATRIX m_ViewMatrix;
	XMMATRIX m_ProjectionMatrix;

	void Create(ComPtr<ID3D12Device2> device, int m_Width, int m_Height)
	{
		// Create the descriptor heap for the depth-stencil view.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

		ResizeDepthBuffer(m_Width, m_Height, m_DepthBuffer, m_DSVHeap);
		
		m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height));

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

	}
};

D3D12Context context;



void RenderSystem::OnAdded()
{
	auto& app = Application::Get();
	context.Create(app.GetDevice(), 1280, 800);

}

void RenderSystem::Start()
{

}

void RenderSystem::Resize(int width, int height)
{

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


void RenderSystem::Draw(D3D12WindowContext& winContext)
{
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList();

	//UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	UINT currentBackBufferIndex = winContext.m_CurrentBackBufferIndex;
	//auto backBuffer = m_pWindow->GetCurrentBackBuffer();
	auto backBuffer = winContext.m_d3d12BackBuffers[currentBackBufferIndex];
	//auto rtv = m_pWindow->GetCurrentRenderTargetView();
	auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(winContext.m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		winContext.m_CurrentBackBufferIndex, winContext.m_RTVDescriptorSize);
	auto dsv = context.m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

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

	commandList->SetPipelineState(context.m_PipelineState.Get());
	commandList->SetGraphicsRootSignature(shader->m_RootSignature.Get());

	m_Scene->ForEach<Renderable>([&commandList, &rtv, &dsv](GameObject* go, Renderable* rend)
	{
		if (rend == nullptr || !rend->m_Enabled || rend->m_Mesh == nullptr)
			return;

		//auto mesh = Mesh::Cube;
		auto mesh = rend->m_Mesh;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &mesh->m_Impl->m_VertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_Impl->m_IndexBufferView);

		commandList->RSSetViewports(1, &context.m_Viewport);
		commandList->RSSetScissorRects(1, &context.m_ScissorRect);

		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		// Update the MVP matrix
		XMMATRIX mvpMatrix = XMMatrixMultiply(context.m_ModelMatrix, context.m_ViewMatrix);
		mvpMatrix = XMMatrixMultiply(mvpMatrix, context.m_ProjectionMatrix);
		commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

		commandList->DrawIndexedInstanced(mesh->m_Indices.size(), 1, 0, 0, 0);
	});

	// Present
	{
		TransitionResource(commandList, backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		context.m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

		//currentBackBufferIndex = m_pWindow->Present();
		bool m_VSync = false;
		UINT syncInterval = m_VSync ? 1 : 0;
		UINT presentFlags = winContext.m_IsTearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(winContext.m_dxgiSwapChain->Present(syncInterval, presentFlags));
		winContext.m_CurrentBackBufferIndex = winContext.m_dxgiSwapChain->GetCurrentBackBufferIndex();

		commandQueue->WaitForFenceValue(context.m_FenceValues[currentBackBufferIndex]);
	}
}

void FishEngine::RenderSystem::Update()
{
	//static uint64_t frameCount = 0;
	static double totalTime = 0.0;

	//totalTime += e.ElapsedTime;
	//frameCount++;

	//if (totalTime > 1.0)
	//{
	//	double fps = frameCount / totalTime;

	//	char buffer[512];
	//	sprintf_s(buffer, "FPS: %f\n", fps);
	//	OutputDebugStringA(buffer);

	//	frameCount = 0;
	//	totalTime = 0.0;
	//}

	//totalTime += 0.00166f;

	// Update the model matrix.
	float angle = static_cast<float>(totalTime * 90.0);
	const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
	context.m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

	// Update the view matrix.
	//const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	//const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	//const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	auto cam = Camera::GetEditorCamera();
	auto t = cam->GetTransform();
	auto p = t->GetPosition();
	auto fw = t->GetForward();
	auto up = t->GetUp();
	const XMVECTOR eyePosition = XMVectorSet(p.x, p.y ,p.z, 1);
	const XMVECTOR toDirection = XMVectorSet(fw.x, fw.y, fw.z, 0);
	const XMVECTOR upDirection = XMVectorSet(up.x, up.y, up.z, 0);
	
	context.m_ViewMatrix = XMMatrixLookToLH(eyePosition, toDirection, upDirection);
	//context.m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	// Update the projection matrix.
	//float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
	float aspectRatio = 1280.f / 800.f;
	float m_FoV = 45.0f;
	context.m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}
