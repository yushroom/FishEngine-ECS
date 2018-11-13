#pragma once

#include "Engine.hpp"
#include "ClassDef.hpp"
#include "Math.hpp"

//#include <bgfx/bgfx.h>

namespace FishEngine
{

	class Mesh;
	class Material;

	class Graphics : public Static
	{
	public:
		static void StaticInit();
		static void BeginFrame();
		static void EndFrame();
		
		static void DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, int submeshID = -1);

		static void DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state, int submeshID = -1);
	};

}
