#include <Material.hpp>
#include <Mesh.hpp>
#include <GameApp.hpp>
#include <ECS.hpp>
#include <Camera.hpp>
#include <Light.hpp>
#include <Renderable.hpp>

#include <GLFW/glfw3.h>

#include <InputSystem.hpp>

const float PI = acosf(-1.0f);

class FreeCameraController : public ECS::ISystem
{
public:
	virtual void Update() override
	{
		Camera* cam = m_Scene->FindComponent<Camera>();
		ECS::GameObject* go = m_Scene->GetGameObjectByID(cam->entityID);
		ECS::Transform* t = go->GetTransform();

		auto& camraPos = t->position;

		auto si = m_Scene->GetSingletonComponent<SingletonInput>();

		if (si->IsButtonPressed(KeyCode::MouseMiddleButton))
		{
			m_MousePos = si->GetMousePosition();
			m_CameraPos = camraPos;
		}
		else if (si->IsButtonHeld(KeyCode::MouseMiddleButton))
		{
			//m_Rad += 0.05f;
			//t->position.x = 10.0f * cosf(m_Rad);
			//t->position.z = 10.0f * sinf(m_Rad);
			Vector2 mousePos2 = si->GetMousePosition();
			Vector2 delta = mousePos2 - m_MousePos;
			//printf("delta: x=%f, y=%f\n", delta.x, delta.y);
			
			delta.x = delta.x / 640.f * 10;
			delta.y = delta.y / 480.f * 10;

			camraPos.x = m_CameraPos.x - delta.x;
			camraPos.y = m_CameraPos.y + delta.y;
		}

//		m_Dictance = Vector3::Distance(cam->lookAt, camraPos);

		if (si->IsButtonPressed(KeyCode::MouseRightButton))
		{
			float m_Rad = -0.5f * PI;
		}
		
		if (si->IsButtonHeld(KeyCode::MouseRightButton) && si->IsButtonHeld(KeyCode::LeftCommand))
		{
			printf("here\n");
		}
	}

private:

	float m_Rad = -0.5f *PI;	// rotate
	float m_Dictance = 0;		// rotate / scale

	// translate
	Vector2 m_MousePos;
	Vector3 m_CameraTarget;
	Vector3 m_CameraPos;
};

class Demo1 : public GameApp
{
public:
	void Start() override
	{
		m_Shader = ShaderUtil::Compile("D:/program/FishEngine-ECS/shader/vs.bin", "D:/program/FishEngine-ECS/shader/fs.bin");
		
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			auto camera = m_Scene->GameObjectAddComponent<Camera>(goID);
			auto go = m_Scene->GetGameObjectByID(goID);
			go->GetTransform()->position.Set(0, 0, -10);
		}
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			auto light = m_Scene->GameObjectAddComponent<Light>(goID);
			light->direction.z = 1;
		}
		
		ECS::EntityID goID = m_Scene->CreateGameObject();
		Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
		rend->mesh = Mesh::Sphere;

		Material* mat = new Material();
		mat->m_Shader = m_Shader;
		mat->pbrparams[0] = 0;
		mat->pbrparams[1] = 0.5f;
		rend->material = mat;

		m_Scene->AddSystem(new FreeCameraController());

		m_Scene->Start();
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
