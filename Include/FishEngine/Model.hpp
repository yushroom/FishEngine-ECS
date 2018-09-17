#pragma once

#include "ClassDef.hpp"
#include <FishEngine/Math.hpp>
#include <vector>
#include <string>

namespace FishEngine
{
	
	class Mesh;
	class Skin;
	class GameObject;
	class Scene;

	class ModelUtil : public Static
	{
	public:
		static GameObject* FromGLTF(const std::string& filePath, Scene* scene);
	};

}
