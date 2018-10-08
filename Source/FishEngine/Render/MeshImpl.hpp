#pragma once

namespace FishEngine
{
	class MeshImpl
	{
		friend class Mesh;
	public:
		// Vertex buffer for the cube.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		// Index buffer for the cube.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};
}