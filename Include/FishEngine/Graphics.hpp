#pragma once

#include "Engine.hpp"
#include "ClassDef.hpp"
#include "Math.hpp"

class Mesh;
class Material;

class Graphics : public Static
{
public:
    static void DrawMesh(Mesh* mesh, const Matrix4x4& matrix, Material* material);
};
