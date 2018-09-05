#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Systems/AnimationSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Assets.hpp>
#include <FishEngine/Components/Animation.hpp>

#include <GLFW/glfw3.h>

class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);

		//const char* test_path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		auto test_path = R"(D:\program\glTF-Sample-Models\2.0\CesiumMan\glTF-Binary\CesiumMan.glb)";
//		auto test_path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/RiggedSimple/glTF-Binary/RiggedSimple.glb";
		Model model = ModelUtil::FromGLTF(test_path, m_Scene);

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -2);
			m_Scene->GameObjectAddComponent<FreeCamera>(go);
		}
		{
			auto go = m_Scene->CreateGameObject();
			auto t = go->GetTransform();
			auto light = m_Scene->GameObjectAddComponent<Light>(go);
			t->SetLocalEulerAngles(50, -30, 0);
			t->SetLocalPosition(0, 3, 0);
		}

		Material* mat = new Material();
		mat->SetShader(m_Shader);
		Vector4 pbrparams(0, 0.5f, 0, 0);
		mat->SetVector("BaseColor", Vector4::one);
		mat->SetVector("PBRParams", pbrparams);

		for (auto go : model.nodes)
		{
			Renderable* r = go->GetComponent<Renderable>();
			if (r != nullptr)
			{
				r->material = mat;
			}
			else
			{
				r = m_Scene->GameObjectAddComponent<Renderable>(go);
				r->mesh = Mesh::Cube;
				r->material = mat;
			}
		}

		model.rootGameObject->GetTransform()->SetLocalEulerAngles(-90, -90, 0);
		//model.rootGameObject->GetTransform()->SetLocalScale(0.01);

		m_Scene->AddSystem(new FreeCameraSystem());
		m_Scene->AddSystem(new AnimationSystem());
	}
	
private:
	Shader* m_Shader = nullptr;
};


int main(void)
{
	ModelViewer demo;
	demo.Run();
	return 0;
}
