#pragma once
#include <cstdint>
#include <type_traits>
#include <functional>
#include <queue>
#include <FishEngine/Utils/EnumFlags.hpp>
#include "../ClassDef.hpp"

#include "Handle.hpp"

namespace FishEngine
{
	enum class RenderReset
	{
		None = 1,
		VSYNC = 2,
		Flag1 = 4,
		Flag2 = 8,
		Flag3 = 16,
	};
	GEN_ENUM_FLAGS(RenderReset);

	typedef std::function<void(void)> CopyTask;
	typedef std::function<void(void)> RenderTask;

	VertexBufferHandle CreateVertexBuffer(const void* data, int vertexCount, int vertexSize);
	IndexBufferHandle CreateIndexBuffer(const void* data, int indexCount, int stride);
	
	struct ViewId {};
	void SetViewRect(ViewId viewId, int x, int y, int width, int height);

	class RenderContext : public Singleton
	{
	public:

		static RenderContext& GetInstance()
		{
			static RenderContext inst;
			return inst;
		}

		void Init();
		void Reset(int width, int height, RenderResetFlags flags);
		void SetViewClear();
		void SetUniform();
		void SetViewTransform();

		void AddCopyQueue(CopyTask copyTask);
		void AddRenderQueue();

		void Frame();

	protected:
		RenderContext();

	private:
		std::queue<CopyTask> m_copyQueue;
	};
}