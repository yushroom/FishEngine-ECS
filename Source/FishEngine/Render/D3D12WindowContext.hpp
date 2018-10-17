#include "../../../include/FishEngine/ClassDef.hpp"

namespace FishEngine
{
	struct D3D12WindowContext : public NonCopyable
	{
		// D3D12
		static constexpr int BufferCount = 3;
		bool m_VSync = true;
		ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
		ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap;
		ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];
		UINT m_RTVDescriptorSize;
		UINT m_CurrentBackBufferIndex;
		bool m_IsTearingSupported;

		void Create(int width, int height, HWND hWnd);
		void Resize(int width, int height);
	};

}