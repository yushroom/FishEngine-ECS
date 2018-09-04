#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Assets.hpp>
#include <FishEngine/Components/Animator.hpp>

#include <GLFW/glfw3.h>

class AnimationSystem : public ECS::ISystem
{
public:
	void OnAdded() override
	{
	}

	void Update() override
	{
		float time = glfwGetTime();
		m_Scene->ForEach<Animation>([time](ECS::GameObject* go, Animation* animation)
		{
			float tt = time;
			while (tt > animation->length)
				tt -= animation->length;
			for (auto& curve : animation->curves)
			{
				auto t = curve.node->GetTransform();
				if (curve.type == AnimationCurveType::Translation)
				{
					Vector3 p = curve.SampleVector3(tt, Vector3::zero);
					t->SetLocalPosition(p);
				}
				else if (curve.type == AnimationCurveType::Rotation)
				{
					Quaternion q = curve.SampleQuat(tt, Quaternion::identity);
					t->SetLocalRotation(q);
				}
				else if (curve.type == AnimationCurveType::Scale)
				{
					Vector3 s = curve.SampleVector3(tt, Vector3::one);
					t->SetLocalScale(s);
				}
			}
		});
	}
};

class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);

		const char* test_path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
//		auto test_path = R"(D:\program\glTF-Sample-Models\2.0\CesiumMan\glTF-Binary\CesiumMan.glb)";
//		auto test_path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/RiggedSimple/glTF-Binary/RiggedSimple.glb";
		Model model = ModelUtil::FromGLTF(test_path, m_Scene);

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -5);
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
//		model.rootGameObject->GetTransform()->SetLocalScale(100);

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
