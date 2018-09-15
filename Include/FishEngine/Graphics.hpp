#pragma once

#include "Engine.hpp"
#include "ClassDef.hpp"
#include "Math.hpp"

#include <bgfx/bgfx.h>

class Mesh;
class Material;

class Graphics : public Static
{
public:
    static void DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material, bgfx::ViewId id=0, int submeshID = -1);

	static void DrawMesh2(Mesh* mesh, const Matrix4x4& matrix, Material* material, uint64_t state, bgfx::ViewId id=0, int submeshID = -1);
};
