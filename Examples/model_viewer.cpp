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
#include <FishEngine/Model.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>

#include <GLFW/glfw3.h>

class Gizmos : public Static
{
public:
	inline static Vector4 color;
	inline static Matrix4x4 matrix;
	
	inline static Material* material = nullptr;
	
	static void Init()
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/color_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/color_fs.bin";
		auto shader = ShaderUtil::Compile(vs, fs);
		material = new Material;
		material->SetShader(shader);
	}
	
	static void DrawCube(const Vector3& center, const Vector3& size)
	{
		auto m = Matrix4x4::Translate(center) * matrix * Matrix4x4::Scale(size);
		material->SetVector("u_color", color);
		Graphics::DrawMesh(Mesh::Cube, m, material);
	}
	
	static void DrawLine(const Vector3& from, const Vector3& to) { }
	static void DrawWireSphere(const Vector3& center, float radius) { }
	
};

class DrawSkeletonSystem : public ECS::ISystem
{
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
//				auto p = t->GetPosition();
//				Gizmos::DrawWireSphere(p, 0.1f);
				Gizmos::matrix = t->GetLocalToWorldMatrix();
				Gizmos::DrawCube(Vector3::zero, Vector3::one * 0.05f);
			}
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
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);

		//const char* path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		auto path = GetglTFSample("CesiumMan");
//		path = GetglTFSample("RiggedSimple");
		path = GetglTFSample("TextureCoordinateTest");
//		path = R"(D:\program\glTF-Sample-Models\2.0\Sponza\glTF\Sponza.gltf)";
//		path = "/Users/yushroom/program/github/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf";
		auto rootGO = ModelUtil::FromGLTF(path, m_Scene);

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

		m_Scene->ForEach<Renderable>([mat](ECS::GameObject* go, Renderable* r){
			if (r->material == nullptr)
				r->material = mat;
		});

		//rootGO->GetTransform()->SetLocalEulerAngles(-90, -90, 0);
//		rootGO->GetTransform()->SetLocalScale(0.01f);

		Gizmos::Init();
		
		m_Scene->AddSystem(new FreeCameraSystem());
		m_Scene->AddSystem(new AnimationSystem());
		m_Scene->AddSystem(new DrawSkeletonSystem());
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
