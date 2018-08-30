#pragma once
#include "ECS.hpp"

class Material;
class Mesh;

class Renderable : public ECS::Component
{
	COMPONENT(Renderable);
	
public:
	Material* material = nullptr;
	Mesh* mesh = nullptr;
};
