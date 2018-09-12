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
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Model.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Systems/SelectionSystem.hpp>

#include <GLFW/glfw3.h>


class DrawSkeletonSystem : public ECS::ISystem
{
	SYSTEM(DrawSkeletonSystem);
public:
	void Update() override
	{
		m_Scene->ForEach<Renderable>([](ECS::GameObject* go, Renderable* rend)
		{
			if (rend->skin == nullptr)
				return;
			
			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::color = Vector4(1, 0, 0, 1);
			for (auto* bone : rend->skin->joints)
			{
				auto t = bone->GetTransform();
				Gizmos::matrix = t->GetLocalToWorldMatrix();
				Gizmos::DrawCube(Vector3::zero, Vector3::one * 0.5f);
				
				//auto p = t->GetParent();
				//if (p != nullptr)
				//{
				//	Gizmos::matrix = p->GetLocalToWorldMatrix();
				//	Gizmos::DrawLine(t->GetLocalPosition(), Vector3::zero);
				//}
			}

			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::color = Vector4(0, 1, 0, 1);
			for (auto* bone : rend->skin->joints)
			{
				auto t = bone->GetTransform();
				auto p = t->GetParent();
				if (p != nullptr)
				{
					Gizmos::DrawLine(t->GetPosition(), p->GetPosition());
				}
			}
		});
	}
};


inline std::string GetglTFSample(const std::string& name)
{
#ifdef __APPLE__
	return "/Users/yushroom/program/github/glTF-Sample-Models/2.0/"
	
#else
	return R"(D:\program\glTF-Sample-Models\2.0\)"
#endif
		+ name + "/glTF-Binary/" + name + ".glb";
}

class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		const char* path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		//auto path = GetglTFSample("CesiumMan");
//		path = GetglTFSample("RiggedSimple");
//		path = GetglTFSample("TextureCoordinateTest");
//		path = GetglTFSample("Triangle");
//		path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/Triangle/glTF/Triangle.gltf";
//		path = R"(D:\program\glTF-Sample-Models\2.0\Sponza\glTF\Sponza.gltf)";
//		path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf";
//		path = GetglTFSample("Buggy");
		auto rootGO = ModelUtil::FromGLTF(path, m_Scene);

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -2);
			m_Scene->GameObjectAddComponent<FreeCamera>(go);
			go->m_Name = "Main Camera";
		}
		{
			auto go = m_Scene->CreateGameObject();
			auto t = go->GetTransform();
			auto light = m_Scene->GameObjectAddComponent<Light>(go);
			t->SetLocalEulerAngles(50, -30, 0);
			t->SetLocalPosition(0, 3, 0);
			go->m_Name = "Directional Light";
		}
		
//		auto rootGO = m_Scene->CreateGameObject();
//		auto r = m_Scene->GameObjectAddComponent<Renderable>(rootGO);
//		r->mesh = Mesh::Cube;
#if 0
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);
		Material* mat = new Material();
		mat->SetShader(m_Shader);
		Vector4 pbrparams(0, 0.5f, 0, 0);
		mat->SetVector("BaseColor", Vector4::one);
		mat->SetVector("PBRParams", pbrparams);
#else
		Material* mat = Material::Clone(Material::ColorMaterial);
		mat->SetVector("u_color", Vector4(1, 1, 0, 1));
#endif
//		r->m_Materials.push_back( mat );

//		m_Scene->ForEach<Renderable>([mat](ECS::GameObject* go, Renderable* r){
//			r->mesh = nullptr;
//		});

		//rootGO->GetTransform()->SetLocalEulerAngles(-90, -90, 0);
		//rootGO->GetTransform()->SetLocalScale(0.1f);

		Gizmos::StaticInit();
		
		m_Scene->AddSystem<FreeCameraSystem>();

		{
			auto s = m_Scene->AddSystem<AnimationSystem>();
			s->m_Priority = 999;
		}
		m_Scene->AddSystem<DrawSkeletonSystem>();

		auto s=  m_Scene->AddSystem<SelectionSystem>();
//		s->selected = rootGO->GetTransform()->GetChildAt(0)->GetChildAt(0)->GetChildAt(0)->m_GameObject;
		s->selected = rootGO;
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
