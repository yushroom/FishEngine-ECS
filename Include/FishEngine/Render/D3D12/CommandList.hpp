#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <memory>

class UploadBuffer;
class ResourceStateTracker;

//namespace FishEngine
//{
	class CommandList
	{
	public:
		CommandList(D3D12_COMMAND_LIST_TYPE type);

		/**
		 * Get the type of command list.
		 */
		D3D12_COMMAND_LIST_TYPE GetCommandListType() const
		{
			return m_d3d12CommandListType;
		}

		/**
		 * Get direct access to the ID3D12GraphicsCommandList2 interface.
		 */
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const
		{
			return m_d3d12CommandList;
		}


		/**
		 * Flush any barriers that have been pushed to the command list.
		 */
		void FlushResourceBarriers();

		void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
		void SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);

	private:
		D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

		// Resource created in an upload heap. Useful for drawing of dynamic geometry
		// or for uploading constant buffer data that changes every draw call.
		std::unique_ptr<UploadBuffer> m_UploadBuffer;

		// Resource state tracker is used by the command list to track (per command list)
		// the current state of a resource. The resource state tracker also tracks the 
		// global state of a resource in order to minimize resource state transitions.
		std::unique_ptr<ResourceStateTracker> m_ResourceStateTracker;
	};
//}