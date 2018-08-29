#pragma once
#include "ECS.hpp"

class Material;
class Mesh;

class Renderable : public Component
{
	COMPONENT(Renderable);
	
public:
	Material* material = nullptr;
	Mesh* mesh = nullptr;
};
