#include <FishEngine/Render/D3D12/D3D12Context.hpp>
#include <FishEngine/Render/D3D12/Application.h>
#include <FishEngine/Render/D3D12/Helpers.h>
#include <FishEngine/Render/CommandQueue.h>
#include <FishEngine/Render/D3D12/d3dx12.h>

using namespace FishEngine;



#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Render/ShaderImpl.hpp>

#include <FishEngine/Render/D3D12/D3D12Utils.hpp>


void FishEngine::D3D12Context::Create(ComPtr<ID3D12Device2> device, int m_Width, int m_Height)
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

		pipelineStateStream.pRootSignature = m_RootSignature.GetRootSignature().Get();
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
