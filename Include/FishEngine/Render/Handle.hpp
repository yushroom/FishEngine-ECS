#pragma once

namespace FishEngine
{
	struct Handle
	{
		Handle() = default;
		Handle(uint16_t idx) : idx(idx) {}
		bool IsValid() const { return idx != 0; }
		uint16_t Get() const { return idx; }

	private:
		uint16_t idx = 0;
	};

	struct VertexBufferHandle : public Handle
	{
		VertexBufferHandle() = default;
		VertexBufferHandle(uint16_t idx) : Handle(idx) {}
	};

	struct IndexBufferHandle : public Handle
	{
		IndexBufferHandle() = default;
		IndexBufferHandle(uint16_t idx) : Handle(idx) {}
	};
}