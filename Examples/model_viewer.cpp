#include <FishEngine.hpp>
#include <FishEditor.hpp>
#include <GLFW/glfw3.h>

using namespace FishEngine;
using namespace FishEditor;

class DrawSkeletonSystem : public System
{
	SYSTEM(DrawSkeletonSystem);
public:
	void Update() override
	{
		m_Scene->ForEach<Renderable>([](GameObject* go, Renderable* rend)
		{
			if (rend->m_Skin == nullptr)
				return;
			
			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::color = Vector4(1, 0, 0, 1);
			for (auto* bone : rend->m_Skin->joints)
			{
				auto t = bone->GetTransform();
				//Gizmos::matrix = t->GetLocalToWorldMatrix();
				//Gizmos::DrawCube(Vector3::zero, Vector3::one * 0.05f);
				Gizmos::DrawCube(t->GetPosition(), Vector3::one * 0.1f);
				
				//auto p = t->GetParent();
				//if (p != nullptr)
				//{
				//	Gizmos::matrix = p->GetLocalToWorldMatrix();
				//	Gizmos::DrawLine(t->GetLocalPosition(), Vector3::zero);
				//}
			}

			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::color = Vector4(0, 1, 0, 1);
			for (auto* bone : rend->m_Skin->joints)
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

GameObject* CreateGO(Scene* scene, Mesh* mesh)
{
	auto go = scene->CreateGameObject();
	go->name = "GameObject";
	auto r = scene->GameObjectAddComponent<Renderable>(go);
	r->m_Mesh = mesh;
	auto mat = Material::Clone(Material::pbrMetallicRoughness);
	mat->SetTexture("baseColorTexture", Texture::s_WhiteTexture);
	r->m_Materials.push_back(mat);
	return go;
}

class ModelViewer : public FishEditor::GameApp
{
public:
	void Start() override
	{
		const char* path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		//auto path = GetglTFSample("CesiumMan");
		//path = GetglTFSample("RiggedSimple");
		//path = GetglTFSample("TextureCoordinateTest");
//		path = GetglTFSample("Triangle");
		//path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf";
//		path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/Triangle/glTF/Triangle.gltf";
		//path = R"(D:\program\glTF-Sample-Models\2.0\Sponza\glTF\Sponza.gltf)";
//		path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf";
//		path = GetglTFSample("Buggy");
		//path = GetglTFSample("BrainStem");

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -10);
			//m_Scene->GameObjectAddComponent<FreeCamera>(go);
			go->name = "Main Camera";
		}
		{
			auto go = m_Scene->CreateGameObject();
			auto t = go->GetTransform();
			auto light = m_Scene->GameObjectAddComponent<Light>(go);
			t->SetLocalEulerAngles(50, -30, 0);
			t->SetLocalPosition(0, 3, 0);
			go->name = "Directional Light";
		}
		
#if 0
		auto plane = CreateGO(m_Scene, Mesh::Plane);
		plane->name = "Plane";
		auto cube = CreateGO(m_Scene, Mesh::Cube);
		cube->name = "Cube";
		auto sphere = CreateGO(m_Scene, Mesh::Sphere);
		sphere->name = "Sphere";
		auto quad = CreateGO(m_Scene, Mesh::Quad);
		quad->name = "Quad";
		auto cone = CreateGO(m_Scene, Mesh::Cone);
		cone->name = "Cone";
		auto cylinder = CreateGO(m_Scene, Mesh::Cylinder);
		cylinder->name = "Cylinder";
		auto capsule = CreateGO(m_Scene, Mesh::Capsule);
		capsule->name = "Capsule";
#endif

		GLTFLoadFlags flags;
		//flags.loadMateirals = false;
		flags.loadPrimitiveAsSubMesh = true;
		auto rootGO = ModelUtil::FromGLTF(path, flags, m_Scene);
//		auto rootGO = m_Scene->CreateGameObject();
//		auto r = m_Scene->GameObjectAddComponent<Renderable>(rootGO);
//		r->mesh = Mesh::Cube;
//		rootGO->GetTransform()->SetLocalEulerAngles(-90, -90, 0);
		//rootGO->GetTransform()->SetLocalScale(100);

		{
			auto s = m_Scene->AddSystem<AnimationSystem>();
			s->m_Priority = 999;
			s->m_Enabled = false;
		}
		m_Scene->AddSystem<DrawSkeletonSystem>();

		auto selection = m_EditorScene->GetSingletonComponent<SingletonSelection>();
//		selection->selected = Camera::GetMainCamera()->m_GameObject;
		selection->selected = rootGO;
		{
			auto cam = Camera::GetEditorCamera();
			assert(cam != nullptr);
			cam->GetTransform()->SetLocalPosition(5, 6, -0.5);
			cam->GetTransform()->SetLocalEulerAngles(30, -90, 0);
		}
		
//		{
//			auto cam = Camera::GetMainCamera();
//			cam->SetFarClipPlane(40);
//			cam->SetNearClipPlane(5);
//			auto t = Camera::GetMainCamera()->GetTransform();
//			t->SetPosition(-12, 0, -12);
//		}
		
//		bgfx::setViewMode(0, bgfx::ViewMode::Sequential);
	}
};


int main(void)
{
	ModelViewer demo;
	demo.Run();
	return 0;
}
