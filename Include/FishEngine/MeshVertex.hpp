#pragma once
#include "Math.hpp"
//#include <bgfx/bgfx.h>
#include "GraphicsAPI.hpp"

namespace FishEngine
{
	
struct VertexPC
{
	Vector3 position;
	Vector3 color;
};

struct PUNTVertex
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector4 tangent;

	static void StaticInit();;

	static VertexDecl ms_decl;
//	static bgfx::VertexDecl ms_decl;

//	static bgfx::VertexDecl s_P_decl;
//	inline static bgfx::VertexDecl s_PC_decl;
};

}
