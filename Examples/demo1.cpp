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

void RotateAround(ECS::Transform* t, const Vector3& axis, float angle)
{
	auto rotation = Quaternion::AngleAxis(angle, axis);
	t->rotation = rotation * t->rotation;
}

class FreeCameraController : public ECS::ISystem
{
public:
	
	enum class ControlType
	{
		None,
		Move,
		Rotate,
		Orbit,
		Zoom,
	};
	
	virtual void Update() override
	{
		Camera* cam = m_Scene->FindComponent<Camera>();
		ECS::GameObject* go = m_Scene->GetGameObjectByID(cam->entityID);
		ECS::Transform* t = go->GetTransform();
		
		auto& camraPos = t->position;
		
		auto si = m_Scene->GetSingletonComponent<SingletonInput>();
		
		bool alt = si->IsButtonHeld(KeyCode::LeftAlt) || si->IsButtonHeld(KeyCode::RightAlt);
		bool ctrl = si->IsButtonHeld(KeyCode::LeftControl) || si->IsButtonHeld(KeyCode::RightControl);
		bool cmd = si->IsButtonHeld(KeyCode::LeftCommand) || si->IsButtonHeld(KeyCode::RightCommand);
		bool left = si->IsButtonPressed(KeyCode::MouseLeftButton) || si->IsButtonHeld(KeyCode::MouseLeftButton);
		bool right = si->IsButtonPressed(KeyCode::MouseRightButton) || si->IsButtonHeld(KeyCode::MouseRightButton);
		bool middle = si->IsButtonPressed(KeyCode::MouseMiddleButton) || si->IsButtonHeld(KeyCode::MouseMiddleButton);
		float scrollValue = si->GetAxis(Axis::MouseScrollWheel);
		bool scroll = scrollValue != 0.0f;
		
		ControlType type = ControlType::None;
		if (middle || (alt && ctrl && left) || (alt && cmd && left))
			type = ControlType::Move;
		else if (alt && left)
			type = ControlType::Orbit;
		else if (scroll || (alt && right))
			type = ControlType::Zoom;
		else if (right)
			type = ControlType::Rotate;
		
		if (type == ControlType::Move)
		{
			float x = m_dragSpeed * si->GetAxis(Axis::MouseX) / 640;
			float y = m_dragSpeed * si->GetAxis(Axis::MouseY) / 480;
			t->Translate({-x, y, 0});
		}
		else if (type == ControlType::Rotate)
		{
			float x = m_rotateSpeed * si->GetAxis(Axis::MouseX) / 640;
			float y = m_rotateSpeed * si->GetAxis(Axis::MouseY) / 480;
			auto pivot = t->position;
			t->RotateAround(pivot, Vector3::up, x);
			t->RotateAround(pivot, t->GetRight(), y);
		}
		else if (type == ControlType::Orbit)
		{
			float x = m_rotateSpeed * si->GetAxis(Axis::MouseX) / 640;
			float y = m_rotateSpeed * si->GetAxis(Axis::MouseY) / 480;
			t->RotateAround(m_orbitCenter, Vector3::up, x);
			t->RotateAround(m_orbitCenter, t->GetRight(), y);
		}
		else if (type == ControlType::Zoom)
		{
			auto forward = t->GetForward();
			float deltaZ = 0;
			if (scrollValue != 0.f)
			{
				deltaZ = 0.2f*scrollValue;
			}
			else
			{
				float x = m_dragSpeed * si->GetAxis(Axis::MouseX) / 640;
				float y = m_dragSpeed * si->GetAxis(Axis::MouseY) / 480;
				deltaZ = fabsf(x) > fabsf(y) ? x : -y;
			}
			t->Translate(deltaZ*forward);
		}
	}
	
private:
	
	bool m_lookAtMode = false;
	float m_rotateSpeed = 200;
	float m_dragSpeed = 20;
	Vector3 m_orbitCenter = Vector3::zero;
};

class Demo1 : public GameApp
{
public:
	void Start() override
	{
		m_Shader = ShaderUtil::Compile("/Users/yushroom/program/FishEngine-ECS/shader/vs.bin", "/Users/yushroom/program/FishEngine-ECS/shader/fs.bin");
		
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(goID);
			auto go = m_Scene->GetGameObjectByID(goID);
			go->GetTransform()->position.Set(0, 0, -15);
		}
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(goID);
		}
		
		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				ECS::EntityID goID = m_Scene->CreateGameObject();
				auto go = m_Scene->GetGameObjectByID(goID);
				auto& pos = go->GetTransform()->position;
				pos.x = -7.5f + x * 1.5f;
				pos.y = -7.5f + y * 1.5f;
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
				rend->mesh = Mesh::Sphere;

				Material* mat = new Material();
				mat->m_Shader = m_Shader;
				mat->pbrparams[0] = x * 0.1f;
				mat->pbrparams[1] = y * 0.1f;
				rend->material = mat;
			}
		}
		
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
