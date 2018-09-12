#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/SingletonInput.hpp>

void SelectionSystem::Update()
{
	if (selected != nullptr)
	{
		auto input = m_Scene->GetSingletonComponent<SingletonInput>();
		auto t = selected->GetTransform();
		
		if (!m_Dragging)
		{
			int selectedAxis = -1;
			Ray ray = Camera::GetEditorCamera()->ScreenPointToRay(input->GetMousePosition_Unity());
			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::DrawRay(ray.origin, ray.direction*10);
			
			Ray ray2;
			auto w2l = t->GetWorldToLocalMatrix();
			ray2.origin = w2l.MultiplyPoint(ray.origin);
			ray2.direction = w2l.MultiplyVector(ray.direction).normalized();
			
			float min_dist = 0.005f;
			const Vector3& D = ray2.origin; 	// ray2.origin - {0, 0, 0}
			
			for (int i = 0 ; i < 3; ++i)
			{
				Vector3 dir;
				dir[i] = 1;
				Vector3 N = Vector3::Cross(ray2.direction, dir);
				float dist = Vector3::Dot(N, D) / D.magnitude();
				dist = ::fabsf(dist);
//				printf("%f\n", dist);
				if (dist < min_dist)
					selectedAxis = i;
			}
			
			m_SelectedAxis = selectedAxis;
			if (selectedAxis != -1 && input->IsButtonPressed(KeyCode::MouseLeftButton))
			{
//				m_SelectedAxis = selectedAxis;
				__OnDragBegin();
				m_MousePosition = input->GetMousePosition();
			}
		}
		else
		{
			if (input->IsButtonHeld(KeyCode::MouseLeftButton))
			{
				__OnDragMove();
			}
			else if (input->IsButtonReleased(KeyCode::MouseLeftButton))
			{
				__OnDragEnd();
				m_SelectedAxis = -1;
				m_Dragging = false;
			}
		}
		
		
		Gizmos::matrix = t->GetLocalToWorldMatrix();
		for (int i = 0; i < 3; ++i)
		{
			if (m_SelectedAxis == i)
				Gizmos::color = Vector4(1, 1, 0, 1);
			else
			{
				Gizmos::color = Vector4(0, 0, 0, 1);
				Gizmos::color[i] = 1;
			}
			Vector3 p;
			p[i] = 1;
			Gizmos::DrawLine(Vector3::zero, p);
		}

		auto r = selected->GetComponent<Renderable>();
		if (r != nullptr && r->mesh != nullptr)
		{
			Gizmos::DrawBounds(r->mesh->bounds);
		}
	}
}


void SelectionSystem::__OnDragBegin()
{
	m_Dragging = true;
	
	assert(selected != nullptr);
	assert(m_SelectedAxis != -1);
	auto camera = Camera::GetEditorCamera();
	auto world2clip = camera->m_ProjectionMatrix * camera->GetWorldToCameraMatrix();
	auto& ray = m_Ray;
	ray.origin = selected->GetTransform()->GetPosition();
	ray.direction = Vector3::zero;
	ray.direction[m_SelectedAxis] = 1;
	ray.origin = world2clip.MultiplyPoint(ray.origin);	// in cli space
//	float t1 = ray.origin.magnitude();	// distance(ray.origin, {0, 0, 0});
	ray.direction = world2clip.MultiplyVector(ray.direction).normalized();
	
	clip2World = world2clip.inverse();
}

void SelectionSystem::__OnDragMove()
{
	assert(selected != nullptr);
	assert(m_SelectedAxis != -1);
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	Vector2 offset = input->GetMousePosition() - m_MousePosition;
	Vector3 offset3 {offset.x, -offset.y, 0};
	
	if (offset3.magnitude() < 0.01f)
		return;
	
	float t = Vector3::Dot(offset3, m_Ray.direction) / offset3.magnitude();
	auto p = m_Ray.GetPoint(t); 		// in clip space
	p = clip2World.MultiplyPoint(p);	// in world space
	selected->GetTransform()->SetPosition(p);
}

void SelectionSystem::__OnDragEnd()
{
	m_Dragging = false;
}
