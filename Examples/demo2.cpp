#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Assets.hpp>

//#include <GLFW/glfw3.h>

const double PI = std::acos(-1);

class Rotator : public ECS::Component
{
	COMPONENT(Rotator)
public:
	float speed = 2.f;
	Vector3 center;
	Vector3 axis = { 0, 0, 1 };
};

class RotatorSystem : public ECS::ISystem
{
public:
	void Update() override
	{
		//double time = glfwGetTime();
		m_Scene->ForEach<Rotator>([](ECS::GameObject* go, Rotator* rot){
			auto t = go->GetTransform();
			auto parent = t->GetParent();
			if (parent != nullptr)
			{
				t->RotateAround(parent->GetPosition(), rot->axis, rot->speed);
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
		Material* mat = new Material();
		mat->SetShader( m_Shader );
		
		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -15);
			m_Scene->GameObjectAddComponent<FreeCamera>(go);
		}
		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(go);
		}
		
		auto CreateRot = [&](Vector3 position, bool AddRot = true){
			auto go = m_Scene->CreateGameObject();
			go->GetTransform()->SetLocalPosition( position );
			if (AddRot)
			{
				m_Scene->GameObjectAddComponent<Rotator>(go);
			}
			Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(go);
			rend->mesh = Mesh::Cube;
			rend->material = mat;
			mat->SetVector("BaseColor", Vector4::one);
			mat->SetVector("PBRParams", Vector4(0, 1, 0, 0));
			return go;
		};
		
		auto sun = CreateRot({0, 0, 0}, false);
		auto earth = CreateRot({5, 0, 0});
		auto moon = CreateRot({2, 0, 0});
		//m_Scene->GameObjectSetParent(earth, sun);
		//m_Scene->GameObjectSetParent(moon, earth);
		earth->GetTransform()->SetParent(sun->GetTransform());
		moon->GetTransform()->SetParent(earth->GetTransform());

		m_Scene->AddSystem(new RotatorSystem());
		m_Scene->AddSystem(new FreeCameraSystem());
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
