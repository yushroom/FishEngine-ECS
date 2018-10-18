#pragma once

#include "../../ClassDef.hpp"
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using DirectX::XMMATRIX;

namespace FishEngine
{
	class D3D12Context : public NonCopyable
	{
	public:

	public:
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

		void Create(ComPtr<ID3D12Device2> device, int m_Width, int m_Height);
	};
}