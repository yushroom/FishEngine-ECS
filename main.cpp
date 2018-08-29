#define _ITERATOR_DEBUG_LEVEL 0
#include "Graphics.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "GameApp.hpp"
#include "RenderSystem.hpp"
#include "ECS.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <GLFW/glfw3.h>


class Camera : public Component
{
	COMPONENT(Camera);
	
public:
	float at[3]  = { 0.0f, 0.0f,   0.0f };
	float eye[3] = { 0.0f, 0.0f, -15.0f };
//	Matrix view;
//	Matrix proj;
};


class Renderable : public Component
{
	COMPONENT(Renderable);
	
public:
	Material* material = nullptr;
	Mesh* mesh = nullptr;
};

#if 0
class Rotator : public Component
{
	COMPONENT(Rotator);
	
public:
	int x, y;
};


class RotatorSystem : public ISystem
{
public:
	void Update(Scene* scene) override
	{
		double time = glfwGetTime();
		
		scene->ForEach<Rotator>([time](GameObject* go, Rotator* rotator)
		{
			auto& mtx = go->transformMatrix;
			int x = rotator->x;
			int y = rotator->y;
			bx::mtxRotateXY(mtx, time + x*0.21f, time + y*0.37f);
			mtx[12] = -15.0f + x*3.0f;
			mtx[13] = -15.0f + y*3.0f;
			mtx[14] = 0.0f;
		});
	}
};

#else

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
	void Update(Scene* scene) override
	{
		double time = glfwGetTime();
		scene->ForEach<Rotator>([time](GameObject* go, Rotator* rot){
			double rad = rot->speed * time;
			float x = 5.0f * cos(rad);
			float y = 5.0f * sin(rad);
			auto& pos = go->GetTransform()->position;
			pos.x = x;
			pos.y = y;
		});
	}
};

#endif


class RenderSystem2 : public ISystem
{
public:
	void Update(Scene* scene) override
	{
		Camera* camera = scene->FindComponent<Camera>();
		if (camera == nullptr)
			return;
		
		float view[16];
		bx::mtxLookAt(view, camera->eye, camera->at);
		
		float width = GameApp::GetMainApp()->GetWidth();
		float height = GameApp::GetMainApp()->GetHeight();
		float ratio = width / height;
		float proj[16];
		bx::mtxProj(proj, 60.0f, ratio, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, view, proj);
		
		
		scene->ForEach<Renderable>([](GameObject* go, Renderable* rend)
		{
			auto& mtx = go->transformMatrix;
			Graphics::DrawMesh(rend->mesh, mtx, rend->material);
		});
	}
};

class TransformSystem : public ISystem
{
public:
	void Update(Scene* scene) override
	{
		scene->All([scene](GameObject* go){
			EntityID parentID = go->GetParent();
			auto& pos = go->GetTransform()->position;
			if (parentID != 0)
			{
				GameObject* parent = scene->GetGameObjectByID(parentID);
				auto& ppos = parent->GetTransform()->position;
				float x = pos.x + ppos.x;
				float y = pos.y + ppos.y;
				auto& mtx = go->transformMatrix;
				mtx[12] = x;
				mtx[13] = y;
				mtx[14] = 0.0f;
			}
			else
			{
				auto& mtx = go->transformMatrix;
				mtx[12] = pos.x;
				mtx[13] = pos.y;
				mtx[14] = 0.0f;
			}
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
		
		m_Scene = new Scene();
		EntityID goID = m_Scene->CreateGameObject();
		m_Scene->GameObjectAddComponent<Camera>(goID);
		
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
				pos.x = -7.5 + x * 1.5f;
				pos.y = -7.5 + y * 1.5f;
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
#endif
		
		m_Scene->AddSystem(new TransformSystem());
		m_Scene->AddSystem(new RenderSystem2());
		m_Scene->AddSystem(new RotatorSystem());
	}

	void Update() override
	{
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);
		
		m_Scene->Update();
	}
	
private:
	Shader* m_Shader = nullptr;
	Scene* m_Scene = nullptr;
};


int main(void)
{
	Demo1 demo;
	demo.Run();
	return 0;
}
