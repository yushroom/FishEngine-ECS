#pragma once

#include <bgfx/bgfx.h>
#include "Object.hpp"


class Mesh : public Object
{
public:
	bgfx::IndexBufferHandle m_IndexBuffer;
	bgfx::VertexBufferHandle m_VertexBuffer;
	
	inline static Mesh* Cube = nullptr;
	
	static void StaticInit();

	void Bind()
	{
		bgfx::setVertexBuffer(0, m_VertexBuffer);
		bgfx::setIndexBuffer(m_IndexBuffer);
	}
};
