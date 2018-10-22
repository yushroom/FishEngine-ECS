#pragma once

#include "Handle.hpp"

class Mesh
{
protected:
	int 				m_VertexCount = 0;
	VertexBufferHandle 	m_VertexBufferHandle;
	IndexBufferHandle 	m_IndexBufferHandle;
};
