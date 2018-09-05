#pragma once

#include "ClassDef.hpp"
#include <FishEngine/Math.hpp>
#include <vector>
#include <string>

class Mesh;
class Skin;

namespace ECS
{
	class GameObject;
	class Scene;
}

namespace tinygltf
{
	class Model;
}


struct Model
{
	ECS::GameObject* rootGameObject = nullptr;
	std::vector<ECS::GameObject*> nodes;
	std::vector<Mesh*> meshes;
	std::vector<Skin*> skins;
	//tinygltf::Model gltfModel;
};


class ModelUtil : public Static
{
public:
	static ECS::GameObject* FromGLTF(const std::string& filePath, ECS::Scene* scene);
};

