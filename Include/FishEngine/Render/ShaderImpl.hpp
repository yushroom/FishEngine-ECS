#pragma once

#include "D3D12/RootSignature.hpp"

namespace FishEngine
{
	class ShaderImpl
	{
	public:
		ComPtr<ID3DBlob> m_VertexShaderBlob;
		ComPtr<ID3DBlob> m_PixelShaderBlob;
		//ComPtr<ID3D12RootSignature> m_RootSignature;
		RootSignature m_RootSignature;
	};
}