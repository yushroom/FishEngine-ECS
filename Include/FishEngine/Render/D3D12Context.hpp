#pragma once

#include "../ClassDef.hpp"
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace FishEngine
{
	class D3D12Context : public NonCopyable
	{
	public:

	public:
		ComPtr<ID3D12Device2> m_D3D12Device;
	};
}