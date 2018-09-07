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

#include <GLFW/glfw3.h>

template<class T>
size_t Sizeof(const std::vector<T> v)
{
	return sizeof(T) * v.size();
}

class Gizmos : public Static
{
	friend class RenderSystem;
public:
	inline static Vector4 color;
	inline static Matrix4x4 matrix;
	
	inline static Material* material = nullptr;
	
	static void StaticInit()
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/color_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/color_fs.bin";
		auto shader = ShaderUtil::Compile(vs, fs);
		material = new Material;
		material->SetShader(shader);

		Vector3 temp;
		//vb = bgfx::createDynamicVertexBuffer(bgfx::copy(&temp, sizeof(temp)), PUNTVertex::s_P_decl, BGFX_BUFFER_ALLOW_RESIZE);
	}
	
	static void DrawCube(const Vector3& center, const Vector3& size)
	{
		auto m = Matrix4x4::Translate(center) * matrix * Matrix4x4::Scale(size);
		material->SetVector("u_color", color);
		Graphics::DrawMesh(Mesh::Cube, m, material);
	}
	

	static void DrawLine(const Vector3& from, const Vector3& to)
	{
		Line line{ from, to };

		auto vb = bgfx::createDynamicVertexBuffer(bgfx::copy(&line, sizeof(line)), PUNTVertex::s_P_decl);

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_PT_LINES);
		material->SetVector("u_color", color);
		material->BindUniforms();
		bgfx::setTransform(matrix.transpose().data());
		bgfx::setVertexBuffer(0, vb);
		bgfx::submit(0, material->GetShader()->GetProgram());
		bgfx::destroy(vb);
	}

	static void DrawWireSphere(const Vector3& center, float radius) { }
	
private:

	struct Line
	{
		Vector3 from;
		Vector3 to;
	};

	inline static std::vector<Line> lines;
};

class SelectionSystem : public ECS::ISystem
{
public:
	void Update() override
	{
		if (selected != nullptr)
		{
			Gizmos::matrix = selected->GetTransform()->GetLocalToWorldMatrix();
			Gizmos::color = Vector4(1, 0, 0, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{1, 0, 0});
			Gizmos::color = Vector4(0, 1, 0, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 1, 0 });
			Gizmos::color = Vector4(0, 0, 1, 1);
			Gizmos::DrawLine(Vector3::zero, Vector3{ 0, 0, 1 });
		}
	}

	ECS::GameObject* selected = nullptr;
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
				Gizmos::matrix = t->GetLocalToWorldMatrix();
				Gizmos::DrawCube(Vector3::zero, Vector3::one * 0.05f);
				
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
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);

		//const char* path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		auto path = GetglTFSample("CesiumMan");
		//auto path = GetglTFSample("RiggedSimple");
		//path = GetglTFSample("Buggy");
		//path = R"(D:\program\glTF-Sample-Models\2.0\Sponza\glTF\Sponza.gltf)";
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

		rootGO->GetTransform()->SetLocalEulerAngles(-90, -90, 0);
		//rootGO->GetTransform()->SetLocalScale(0.1f);

		Gizmos::StaticInit();
		
		m_Scene->AddSystem(new FreeCameraSystem());

		{
			auto s = new AnimationSystem();
			m_Scene->AddSystem(s);
			s->m_Priority = 999;
		}
		//m_Scene->AddSystem(new DrawSkeletonSystem());

		auto s = new SelectionSystem();
		m_Scene->AddSystem(s);
		s->selected = rootGO->GetTransform()->GetChildAt(0)->GetChildAt(0)->GetChildAt(0)->m_GameObject;
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
