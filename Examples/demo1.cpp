#include <Material.hpp>
#include <Mesh.hpp>
#include <GameApp.hpp>
#include <ECS.hpp>
#include <Camera.hpp>
#include <Light.hpp>
#include <Renderable.hpp>

#include <GLFW/glfw3.h>


const double PI = std::acos(-1);

class Rotator : public Component
{
	COMPONENT(Rotator)
public:
	float speed = 2.f;
	Vector3 center;
};

class RotatorSystem : public ISystem
{
public:
	void Update() override
	{
		double time = glfwGetTime();
		m_Scene->ForEach<Rotator>([time](GameObject* go, Rotator* rot){
			float rad = rot->speed * time;
			float x = 5.0f * cosf(rad);
			float y = 5.0f * sinf(rad);
			auto& pos = go->GetTransform()->position;
			pos.x = x;
			pos.y = y;
		});
	}
};


class Demo1 : public GameApp
{
public:
	void Start() override
	{
		m_Shader = ShaderUtil::Compile("D:/program/FishEngine-ECS/shader/vs.bin", "D:/program/FishEngine-ECS/shader/fs.bin");

		Material* mat = new Material();
		mat->m_Shader = m_Shader;
		
		{
			EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(goID);
		}
		{
			EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(goID);
		}
		
#if 1
		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				EntityID goID = m_Scene->CreateGameObject();
				//Rotator* rotator = m_Scene->GameObjectAddComponent<Rotator>(goID);
				//rotator->x = x;
				//rotator->y = y;
				auto go = m_Scene->GetGameObjectByID(goID);
				auto& pos = go->GetTransform()->position;
				pos.x = -7.5f + x * 1.5f;
				pos.y = -7.5f + y * 1.5f;
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
				rend->mesh = Mesh::Sphere;
				rend->material = mat;
			}
		}
#else
		auto CreateRot = [&](bool AddRot = true){
			EntityID id = m_Scene->CreateGameObject();
			if (AddRot)
			{
				m_Scene->GameObjectAddComponent<Rotator>(id);
			}
			Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(id);
			rend->mesh = Mesh::Cube;
			rend->material = mat;
			return id;
		};
		
		auto sun = CreateRot(false);
		auto earth = CreateRot();
		auto moon = CreateRot();
		m_Scene->GameObjectSetParent(earth, sun);
		m_Scene->GameObjectSetParent(moon, earth);
		m_Scene->AddSystem(new RotatorSystem());
#endif
		m_Scene->Start();
	}

	void Update() override
	{
		float lightDir[] = { 1, 1, 1, 0 };
		m_Scene->Update();
	}
	
private:
	Shader* m_Shader = nullptr;
};


int main(void)
{
	Demo1 demo;
	demo.Run();
	return 0;
}
