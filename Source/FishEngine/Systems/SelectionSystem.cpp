#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/SingletonInput.hpp>

void SelectionSystem::Update()
{
	if (selected == nullptr)
	{
		return;
	}
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	auto t = selected->GetTransform();
	auto camera = Camera::GetEditorCamera();

	Vector3 axes[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	bool show_axis[3] = { true, true, true };

	for (auto& a : axes)
		a = a.normalized();

	auto view = camera->GetTransform()->GetForward().normalized();
	for (int i = 0; i < 3; ++i)
	{
		if (Mathf::CompareApproximately(Mathf::Abs(Vector3::Dot(view, axes[i])), 1, 1e-3))	// parallel
		{
			show_axis[i] = false;
		}
	}

	if (m_EnableTransform)
	{
		if (!m_Dragging)
		{
			int selectedAxis = -1;
			Ray ray = camera->ScreenPointToRay(input->GetMousePosition_Unity());
			Gizmos::matrix = Matrix4x4::identity;
			Gizmos::DrawRay(ray.origin, ray.direction * 10);

			Ray ray2;
			auto w2l = t->GetWorldToLocalMatrix();
			ray2.origin = w2l.MultiplyPoint(ray.origin);
			ray2.direction = w2l.MultiplyVector(ray.direction).normalized();

			float min_dist = 0.05f;
			const Vector3& D = ray2.origin; 	// D = ray2.origin - {0, 0, 0}

			for (int i = 0; i < 3; ++i)
			{
				if (!show_axis[i])
					continue;
				auto& axis = axes[i];
				Vector3 N = Vector3::Cross(ray2.direction, axis).normalized();
				float dist = Vector3::Dot(N, D);
				dist = ::fabsf(dist);
//				printf("%f\n", dist);
				if (dist < min_dist)
					selectedAxis = i;
			}

			m_SelectedAxis = selectedAxis;
			if (selectedAxis != -1 && input->IsButtonPressed(KeyCode::MouseLeftButton))
			{
				m_MousePosition = input->GetMousePosition();	// save old mouse position
				__OnDragBegin();
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
	}
	else
	{
		m_SelectedAxis = -1;
	}
		
	
	// draw translation gizmo
	for (int i = 0; i < 3; ++i)
	{
		if (!show_axis[i])
			continue;

		if (m_SelectedAxis == i)
		{
			Gizmos::color = Vector4(1, 1, 0, 1);
		}
		else
		{
			if (m_SelectedAxis != -1 && m_Dragging)	// grey other axis when dragging
			{
				Gizmos::color = Vector4(0.5f, 0.5f, 0.5f, 1);
			}
			else
			{
				Gizmos::color = Vector4(0, 0, 0, 1);
				Gizmos::color[i] = 1;
			}
		}
		Vector3 p{0, 0, 0};
		p[i] = 1;

		// triangle similarity
		constexpr float fixed_size = 0.4f;		// fixed_size on screen
		float depth = Vector3::Distance(camera->GetTransform()->GetPosition(), t->GetPosition());
		float depth_screen = 1.0f / ::tanf(Mathf::Radians(camera->m_FOV/2.f));
		float scale = fixed_size * depth / depth_screen;

		Gizmos::matrix = t->GetLocalToWorldMatrix() * Matrix4x4::Scale(scale);
		Gizmos::DrawLine(Vector3::zero, p);
	}

	// draw bbox
	auto r = selected->GetComponent<Renderable>();
	if (r != nullptr && r->mesh != nullptr)
	{
		Gizmos::DrawBounds(r->mesh->bounds);
	}
}


void SelectionSystem::__OnDragBegin()
{
	m_Dragging = true;
	
	assert(selected != nullptr);
	assert(m_SelectedAxis != -1);
	auto camera = Camera::GetEditorCamera();
	auto world2clip = camera->m_ProjectionMatrix * camera->GetWorldToCameraMatrix();	// projViewMat
	auto& ray = m_Ray;
	auto posW = selected->GetTransform()->GetPosition();
	ray.origin = posW;
	ray.direction = Vector3::zero;
	ray.direction[m_SelectedAxis] = 1;
	ray.direction = ray.direction.normalized();

	// move ray from world to clip space
	ray.origin = world2clip.MultiplyPoint(ray.origin);
#if 0
	// 注意，不能用透视矩阵变换向量
	ray.direction = world2clip.MultiplyVector(ray.direction).normalized();	
#else
	ray.direction = world2clip.MultiplyPoint(ray.direction + posW) - ray.origin;
	ray.direction = Vector3::Normalize(ray.direction);
#endif
	
	m_Clip2World = world2clip.inverse();

#if 0
	Vector3 test = clip2World.MultiplyPoint(ray.GetPoint(1));
	test = test - posW;
	test = test.normalized();
	printf("%s\n", test.ToString().c_str());
	auto test2 = clip2World.MultiplyPoint(ray.origin);
	printf("%s\n", test2.ToString().c_str());
#endif
}

void SelectionSystem::__OnDragMove()
{
	assert(selected != nullptr);
	assert(m_SelectedAxis != -1);
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	Vector2 offset = input->GetMousePosition() - m_MousePosition;
	Vector3 offset3 {offset.x*2.f, offset.y*2.f, 0};		// (-1, 1) in NDC
	
#if 0
	Gizmos::matrix = clip2World;
	Vector3 p1{ m_MousePosition.x*2-1, m_MousePosition.y*2-1, 0.5f };
	Vector3 p2{ input->GetMousePosition().x * 2 - 1, input->GetMousePosition().y * 2 - 1, 0.5f };
	Gizmos::DrawLine(p1, p2);
#endif

	float len = offset3.magnitude();
	if (len < 0.01f)
		return;

	Vector3 d = m_Ray.direction;
	d.z = 0;
	d = d.normalized();
	
	float t = Vector3::Dot(offset3, d);
	auto p = m_Ray.GetPoint(t); 		// in clip space
	p = m_Clip2World.MultiplyPoint(p);	// in world space
	selected->GetTransform()->SetPosition(p);
}

void SelectionSystem::__OnDragEnd()
{
	m_Dragging = false;
}
