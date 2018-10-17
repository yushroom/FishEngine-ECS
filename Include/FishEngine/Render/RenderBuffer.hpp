#pragma once

namespace FishEngine::D3D12
{
	struct VertexBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		D3D12_VERTEX_BUFFER_VIEW bufferView;

		static std::unordered_map<uint16_t, VertexBuffer> Buffers;
		static uint16_t NextIndex;
	};


	struct IndexBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		D3D12_INDEX_BUFFER_VIEW bufferView;

		static std::unordered_map<uint16_t, IndexBuffer> Buffers;
		static uint16_t NextIndex;
	};
}