#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>

#include <glfw/glfw3.h>

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
		double time = glfwGetTime();
		m_Scene->ForEach<Rotator>([this, time](ECS::GameObject* go, Rotator* rot){
			auto t = go->GetTransform();
			auto pid = go->GetParentID();
			if (pid != 0)
			{
				auto parent = m_Scene->GetGameObjectByID(pid);
				t->RotateAround(Vector3::zero, rot->axis, rot->speed);
			}
		});
	}
};


class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		m_Shader = ShaderUtil::Compile("D:/program/FishEngine-ECS/shader/vs.bin", "D:/program/FishEngine-ECS/shader/fs.bin");

		Material* mat = new Material();
		mat->m_Shader = m_Shader;
		
		{
			auto goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(goID);
			auto go = m_Scene->GetGameObjectByID(goID);
			go->GetTransform()->position.Set(0, 0, -15);
			m_Scene->GameObjectAddComponent<FreeCamera>(goID);
		}
		{
			auto goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(goID);
		}
		
		auto CreateRot = [&](Vector3 position, bool AddRot = true){
			auto id = m_Scene->CreateGameObject();
			auto go = m_Scene->GetGameObjectByID(id);
			go->GetTransform()->position = position;
			if (AddRot)
			{
				m_Scene->GameObjectAddComponent<Rotator>(id);
			}
			Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(id);
			rend->mesh = Mesh::Cube;
			rend->material = mat;
			return id;
		};
		
		auto sun = CreateRot({0, 0, 0}, false);
		auto earth = CreateRot({5, 0, 0});
		auto moon = CreateRot({2, 0, 0});
		m_Scene->GameObjectSetParent(earth, sun);
		m_Scene->GameObjectSetParent(moon, earth);
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
