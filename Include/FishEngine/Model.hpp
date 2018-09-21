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

	struct GLTFLoadFlags
	{
		//bool asGameObject;
		bool loadMateirals = true;
		bool loadMeshes = true;
		bool loadNodes = true;
		bool loadCamera = true;
		bool loadPrimitiveAsSubMesh = true;
	};

	class ModelUtil : public Static
	{
	public:
		static std::vector<Mesh*> LoadMeshesFromGLTF(const std::string& filePath);
		static GameObject* FromGLTF(const std::string& filePath, const GLTFLoadFlags& flags, Scene* scene = nullptr);
	};

}
