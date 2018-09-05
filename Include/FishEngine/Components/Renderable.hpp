#pragma once
#include "FishEngine/ECS.hpp"

class Material;
class Mesh;


class Skin : public NonCopyable
{
	//COMPONENT(Skin);
public:
	std::vector<Matrix4x4> inverseBindMatrices;
	ECS::GameObject* root = nullptr;
	std::vector<ECS::GameObject*> joints;
	std::string name;
};


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