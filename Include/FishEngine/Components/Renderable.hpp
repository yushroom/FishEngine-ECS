#pragma once
#include "FishEngine/ECS.hpp"
#include <FishEngine/Mesh.hpp>

class Material;
class Mesh;

class Renderable : public ECS::Component
{
	COMPONENT(Renderable);
	
public:
	Material* material = nullptr;
	Mesh* mesh = nullptr;
	Skin* skin = nullptr;
};


class Skybox : public ECS::Component
{
	COMPONENT(Skybox);
public:
	Material * m_skyboxMaterial = nullptr;
};