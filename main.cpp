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
	float eye[3] = { 0.0f, 0.0f, -35.0f };
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
			auto& mtx = go->transform;
			int x = rotator->x;
			int y = rotator->y;
			bx::mtxRotateXY(mtx, time + x*0.21f, time + y*0.37f);
			mtx[12] = -15.0f + x*3.0f;
			mtx[13] = -15.0f + y*3.0f;
			mtx[14] = 0.0f;
		});
	}
};


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
		
		float ratio = 640.0f / 480;
		float proj[16];
		bx::mtxProj(proj, 60.0f, ratio, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, view, proj);
		
		
		scene->ForEach<Renderable>([](GameObject* go, Renderable* rend)
		 {
			 auto& mtx = go->transform;
			 Graphics::DrawMesh(rend->mesh, mtx, rend->material);
		 });
	}
};


class Demo1 : public GameApp
{
public:
	void Start() override
	{
		m_Shader = ShaderUtil::Compile("/Users/yushroom/program/test_bgfx/shader/vs.bin", "/Users/yushroom/program/test_bgfx/shader/fs.bin");
		
		Material* mat = new Material();
		mat->m_Shader = m_Shader;
		
		m_Scene = new Scene();
		EntityID goID = m_Scene->CreateGameObject();
		m_Scene->GameObjectAddComponent<Camera>(goID);
		
		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				EntityID goID = m_Scene->CreateGameObject();
				Rotator* rotator = m_Scene->GameObjectAddComponent<Rotator>(goID);
				rotator->x = x;
				rotator->y = y;
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
				rend->mesh = Mesh::Cube;
				rend->material = mat;
			}
		}
		
		m_Scene->systems.push_back(new RenderSystem2());
		m_Scene->systems.push_back(new RotatorSystem());
	}

	void Update() override
	{
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);
		
		for (auto& s : m_Scene->systems)
		{
			s->Update(m_Scene);
		}
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
