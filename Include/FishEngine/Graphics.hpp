#pragma once

#include "Engine.hpp"
#include "ClassDef.hpp"

class Mesh;
class Material;

class Graphics : public Static
{
public:
    static void DrawMesh(Mesh* mesh, Matrix matrix, Material* material);
};
