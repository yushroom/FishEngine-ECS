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

class ModelUtil : public Static
{
public:
	static ECS::GameObject* FromGLTF(const std::string& filePath, ECS::Scene* scene);
};

