#pragma once

namespace FishEngine
{
	class ShaderImpl
	{
	public:
		ComPtr<ID3DBlob> m_VertexShaderBlob;
		ComPtr<ID3DBlob> m_PixelShaderBlob;
		ComPtr<ID3D12RootSignature> m_RootSignature;
	};
}