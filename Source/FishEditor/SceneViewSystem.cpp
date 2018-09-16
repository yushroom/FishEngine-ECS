#include <FishEditor/Systems/SceneViewSystem.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Render/RenderViewType.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>

#include <FishEditor/Components/SingletonSelection.hpp>

Material* s_color_material = nullptr;
Vector4 s_selected_axis_color{ 1, 1, 0, 1 };
Vector4 s_axis_colors[3] = {
	{ 1, 0, 0, 1 },
	{ 0, 1, 0, 1 },
	{ 0, 0, 1, 1 }
};

constexpr float s_fixed_pixel_size_of_axis = 100;		// fixed size(in pixels) on screen
inline float get_fixed_axis_size()				// fixed_size on screen
{
	return s_fixed_pixel_size_of_axis / Screen::height * 2;
}
inline float get_gizmo_scale(float depth, float camera_fov)
{
	const float fixed_size = get_fixed_axis_size();
	float depth_screen = 1.0f / ::tanf(Mathf::Radians(camera_fov / 2.f));
	float scale = fixed_size * depth / depth_screen;	// triangle similarity
	return scale;
}

constexpr float s_axis_hover_threshold_pixels = 6;


class TransformGizmo
{
public:
	virtual void Update(TransformSpace space, SingletonInput* input, Camera* camera, Transform* selectedT) = 0;
	
	void OnLMBPressed(const Vector2& mousePos)
	{
		if (m_SelectedAxis != -1)
			__OnDragBegin(mousePos);
	}
	
	void OnLMBHeld(const Vector2& mousePos)
	{
		if (m_Dragging)
			__OnDragMove(mousePos);
	}
	
	void OnLMBReleased()
	{
		m_Dragging = false;
	}
	
	bool IsDragging() const { return m_Dragging; }

protected:
	
	virtual void __OnDragBegin(const Vector2& mousePos) = 0;
	virtual void __OnDragMove(const Vector2& mousePos) = 0;
	
	
	int m_HoveringAxis = -1;
	int m_SelectedAxis = -1;
	bool m_Dragging = false;
	
	TransformSpace m_transformSpace;
	Camera* m_Camera = nullptr;
	Transform* m_SelectedT = nullptr;
};


class TTransformGizmo : public TransformGizmo
{
public:
	void Update(TransformSpace space, SingletonInput* input, Camera* camera, Transform* selectedT) override
	{
		m_transformSpace = space;
		m_Camera = camera;
		m_SelectedT = selectedT;
		
		auto mouse_pos = input->GetMousePosition();
		
		for (int i = 0; i < 3; ++i)
			show_axis[i] = true;
		Vector3 axes[3] = { { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 } };
		if (space == TransformSpace::Local)
		{
			auto l2w = selectedT->GetLocalToWorldMatrix();
			for (auto& a : axes)
				a = l2w.MultiplyVector(a).normalized();
		}
		
		int hovered_axis = -1;
		if (!m_Dragging)
		{
			const auto o = selectedT->GetPosition();
			const auto world2clip = camera->GetViewProjectionMatrix();	// projViewMat
			mouse_pos = mouse_pos * 2 - 1;
			const auto mp = Vector3(mouse_pos.x, mouse_pos.y, 0);		// mouse position on the screen(NDC)
			float min_dist = s_axis_hover_threshold_pixels / Screen::height * 2;
			
			auto axis_origin = world2clip.MultiplyPoint(o);
			axis_origin.z = 0;
			
			const float depth = Vector3::Distance(camera->GetTransform()->GetPosition(), selectedT->GetPosition());
			const float scale = get_gizmo_scale(depth, camera->GetFieldOfView());
			
			for (int i = 0; i < 3; ++i)
			{
				auto axis_end = o + axes[i] * scale;
				axis_end = world2clip.MultiplyPoint(axis_end);
				axis_end.z = 0;
				
				auto axis_dir = axis_end - axis_origin;
				float t_unit = axis_dir.magnitude();
				if (t_unit < 0.05f)	// parallel
				{
					show_axis[i] = false;
					continue;
				}
				
				axis_dir = axis_dir.normalized();		// Ray(axis_origin, axis_dir) is the axis on the screen(NDC)
				
				auto o_mp = mp - axis_origin;
				float t = Vector3::Dot(o_mp, axis_dir);		// Dot(o_mp, axis_dir) / len(axis_dir)
				
				if (t >= 0 && t <= t_unit)
				{
					float dist = Vector3::Distance(mp, axis_dir*t + axis_origin);
					if (dist < min_dist)
					{
						hovered_axis = i;
						min_dist = dist;
					}
				}
			}
			//printf("%f\n", min_dist);
			m_SelectedAxis = hovered_axis;
		}
		
		Draw(space, camera, selectedT);
	}
	
	
	void Draw(TransformSpace space, Camera* camera, Transform* selectedT)
	{
		Quaternion rotations[] = {
			Quaternion::AngleAxis(-90, { 0, 0, 1 }),
			Quaternion::identity,
			Quaternion::AngleAxis(90, { 1, 0, 0 }),
		};
		
		for (int i = 0; i < 3; ++i)
		{
			if (!show_axis[i])
				continue;
			
			Vector4 color = s_axis_colors[i];
			if (m_SelectedAxis == i)
				color = s_selected_axis_color;
			else if (m_SelectedAxis != -1 && m_Dragging)	// grey other axis when dragging
				color = Vector4(0.5f, 0.5f, 0.5f, 1);
			
			Vector3 p{ 0, 0, 0 };
			p[i] = 1;
			
			float depth = Vector3::Distance(camera->GetTransform()->GetPosition(), selectedT->GetPosition());
			float scale = get_gizmo_scale(depth, camera->GetFieldOfView());
			
//			Gizmos::matrix = t->GetLocalToWorldMatrix() * Matrix4x4::Scale(scale);
			auto modelMat = selectedT->GetLocalToWorldMatrix();
			auto pos = modelMat._MultiplyPoint(0, 0, 0);
			if (space == TransformSpace::Global)
				Gizmos::matrix = Matrix4x4::TRS(pos, Quaternion::identity, Vector3::one * scale);
			else
			{
				auto rot = modelMat.ToRotation();
				Gizmos::matrix = Matrix4x4::TRS(pos, rot, Vector3::one * scale);
			}
			Gizmos::color = color;
			Gizmos::DrawLine(Vector3::zero, p);
			
			Quaternion r = rotations[i];
			auto m = Gizmos::matrix * Matrix4x4::TRS(p, r, Vector3(1, 1.5, 1)*0.08f);
			s_color_material->SetVector("u_color", color);
			
			uint64_t state = 0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_ALWAYS
				| BGFX_STATE_CULL_CCW;
			Graphics::DrawMesh2(Mesh::Cone, m, s_color_material, state, (bgfx::ViewId)RenderViewType::SceneGizmos);
		}
	}
	

	
	
	void __OnDragBegin(const Vector2& mousePos) override
	{
		m_MousePositionWhenDraggingStarted = mousePos;	// save old mouse position
		m_Dragging = true;
		
		assert(m_SelectedT != nullptr);
		assert(m_SelectedAxis != -1);
		auto camera = Camera::GetEditorCamera();
		auto world2clip = camera->GetViewProjectionMatrix();	// projViewMat
		auto& ray = m_Ray;
		auto posW = m_SelectedT->GetPosition();
		ray.origin = posW;
		//ray.direction = Vector3::zero;
		//ray.direction[m_SelectedAxis] = 1;
		
		Vector3 axes[3] = { { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 } };
		auto a = axes[m_SelectedAxis];
		if (m_transformSpace == TransformSpace::Local)
		{
			auto l2w = m_SelectedT->GetLocalToWorldMatrix();
			a = l2w.MultiplyVector(a).normalized();
		}
		ray.direction = a.normalized();
		
		// move ray from world to clip space
		ray.origin = world2clip.MultiplyPoint(ray.origin);
		ray.direction = world2clip.MultiplyPoint(ray.direction + posW) - ray.origin;
		ray.direction = Vector3::Normalize(ray.direction);
		m_Clip2World = world2clip.inverse();
	}
	
	
	void __OnDragMove(const Vector2& mousePos) override
	{
		assert(m_SelectedT != nullptr);
		assert(m_SelectedAxis != -1);
		Vector2 offset = mousePos - m_MousePositionWhenDraggingStarted;
		Vector3 offset3{ offset.x*2.f, offset.y*2.f, 0 };		// (-1, 1) in NDC
		
		float len = offset3.magnitude();
		if (len < 0.01f)
			return;
		
		Vector3 d = m_Ray.direction;
		d.z = 0;
		d = d.normalized();
		
		float t = Vector3::Dot(offset3, d);
		auto p = m_Ray.GetPoint(t); 		// in clip space
		p = m_Clip2World.MultiplyPoint(p);	// in world space
		m_SelectedT->SetPosition(p);
	}
	
	
private:
	bool show_axis[3] = { true, true, true };
	Vector2 m_MousePositionWhenDraggingStarted;
	Matrix4x4 m_Clip2World;
	Ray m_Ray;
};


class STransformGizmo : public TransformGizmo
{
	
};


void SceneViewSystem::OnAdded()
{
	s_color_material = Material::Clone(Material::ColorMaterial);
}


void DrawCircle2(const Vector3& cameraPos, const Matrix4x4& m, float depth)
{
//	Gizmos::color = {1, 0, 0, 1};
//	Gizmos::matrix = Matrix4x4::identity;
	float d = depth+0.2;
	for (int i = 0; i < Gizmos::circle_vertex_count-1; ++i)
	{
		auto v1 = Gizmos::vertices[i];
		v1 = m.MultiplyPoint(v1);
		float d1 = Vector3::Distance(cameraPos, v1);
		auto v2 = Gizmos::vertices[i+1];
		v2 = m.MultiplyPoint(v2);
		float d2 = Vector3::Distance(cameraPos, v2);
		if (d1 < d && d2 < d)
		{
			Gizmos::DrawLine(v1, v2);
		}
	}
}

bool RaySphereIntersect(Vector3 o, float R, const Ray& ray, float* out_t)
{
	o = ray.origin - o;
	// solve: || o+dt || = R
	auto d = ray.direction.normalized();
	float C = o.sqrMagnitude() - R*R;
	float B = 2.f * Vector3::Dot(o, d);
	float A = d.sqrMagnitude();
	if (A < 1e-4)
		return false;
	
	float delta = B*B - 4*A*C;
	float sdelta = sqrtf(delta);
	float t0 = (-B+sdelta)/(2*A);
	float t1 = (-B-sdelta)/(2*A);
	if (t0 > t1)
		std::swap(t0, t1);
	// t0 <= t1
	
	float t = t0 > 0 ? t0 : t1;
	if (t > 0)
		*out_t = t;
	return t > 0;
}


class RTransformGizmo : public TransformGizmo
{
public:
	void Update(TransformSpace space, SingletonInput* input, Camera* camera, Transform* selectedT) override
	{
		m_transformSpace = space;
		m_Camera = camera;
		m_SelectedT = selectedT;
		
		auto cameraPos = camera->GetTransform()->GetPosition();
		float depth = Vector3::Distance(cameraPos, selectedT->GetPosition());
		float scale = get_gizmo_scale(depth, camera->GetFieldOfView());
		m_Scale = scale;
		
		//Matrix4x4 view = camera->GetWorldToCameraMatrix();
		Vector3 pos = selectedT->GetPosition();
		Quaternion rot = Quaternion::identity;
		Vector3 s = Vector3::one*scale;
		
		m_HoveringAxis = -1;
		Ray ray = camera->ScreenPointToRay(input->GetMousePosition_Unity());
		float t = 0;
		if (RaySphereIntersect(pos, scale, ray, &t))
		{
			auto i_pos = ray.GetPoint(t);
			auto mat = Matrix4x4::TRS(pos, selectedT->GetLocalToWorldMatrix().ToRotation(), Vector3::one);
			mat = mat.inverse();
			i_pos = mat.MultiplyPoint( i_pos) / scale;
			if (Mathf::CompareApproximately(i_pos.x, 0, 0.1f))
				m_HoveringAxis = 0;
			else if (Mathf::CompareApproximately(i_pos.y, 0, 0.1f))
				m_HoveringAxis = 1;
			else if (Mathf::CompareApproximately(i_pos.z, 0, 0.1f))
				m_HoveringAxis = 2;
		}
		else
		{
			rot = Quaternion::LookRotation(camera->GetTransform()->GetForward());
			Gizmos::color = {1, 1, 1, 1};
			Gizmos::matrix = Matrix4x4::TRS(pos, rot, s);
			Gizmos::DrawCircle(Vector3::zero, 1.1);
		}
		
		Draw();
	}
	
	void Draw()
	{
		auto pos = m_SelectedT->GetPosition();
		Quaternion rot;
		Vector3 s = Vector3::one * m_Scale;
#if 1
		rot = Quaternion::AngleAxis(90, {0, 1, 0});
		Gizmos::color = {1, 0, 0, 1};
		if (m_HoveringAxis == 0)
			Gizmos::color = {1, 1, 0, 1};
		Gizmos::matrix = Matrix4x4::TRS(pos, rot, s);
		Gizmos::DrawCircle(Vector3::zero, 1);
		
		rot = Quaternion::AngleAxis(90, {1, 0, 0});
		Gizmos::color = {0, 1, 0, 1};
		if (m_HoveringAxis == 1)
			Gizmos::color = {1, 1, 0, 1};
		Gizmos::matrix = Matrix4x4::TRS(pos, rot, s);
		Gizmos::DrawCircle(Vector3::zero, 1);
		
		
		rot = Quaternion::identity;
		Gizmos::color = {0, 0, 1, 1};
		if (m_HoveringAxis == 2)
			Gizmos::color = {1, 1, 0, 1};
		Gizmos::matrix = Matrix4x4::TRS(pos, rot, s);
		Gizmos::DrawCircle(Vector3::zero, 1);
#else
		Gizmos::matrix = Matrix4x4::identity;
		
		auto m = Matrix4x4::TRS(pos, rot, s);
		Gizmos::color = {1, 0, 0, 1};
		if (hovered_axis == 0)
			Gizmos::color = {1, 1, 0, 1};
		DrawCircle2(cameraPos, m, depth);
		
		Gizmos::color = {0, 1, 0, 1};
		if (hovered_axis == 1)
			Gizmos::color = {1, 1, 0, 1};
		rot = Quaternion::AngleAxis(90, {0, 1, 0});
		m = Matrix4x4::TRS(pos, rot, s);
		DrawCircle2(cameraPos, m, depth);
		
		Gizmos::color = {0, 0, 1, 1};
		if (hovered_axis == 2)
			Gizmos::color = {1, 1, 0, 1};
		rot = Quaternion::AngleAxis(90, {1, 0, 0});
		m = Matrix4x4::TRS(pos, rot, s);
		DrawCircle2(cameraPos, m, depth);
#endif

	}
	
	void __OnDragBegin(const Vector2& mousePos) override
	{
	}
	
	void __OnDragMove(const Vector2& mousePos) override
	{
		
	}
	
private:
	float m_Scale = 1.0f;
};



void SceneViewSystem::DrawGizmos()
{
	auto selected = m_Scene->GetSingletonComponent<SingletonSelection>()->selected;
	if (selected == nullptr)
	{
		return;
	}
	auto input = m_Scene->GetSingletonComponent<SingletonInput>();
	auto selectedT = selected->GetTransform();
	auto camera = Camera::GetEditorCamera();
	auto mousePos = input->GetMousePosition();
	
	static TTransformGizmo tg;
	static RTransformGizmo rg;
	
	if (m_transformToolType == TransformToolType::Translate)
		tg.Update(m_transformSpace, input, camera, selectedT);
	else if (m_transformToolType == TransformToolType::Rotate)
		rg.Update(m_transformSpace, input, camera, selectedT);
	
	
	if (m_EnableTransform)
	{
		if (input->IsButtonPressed(KeyCode::MouseLeftButton))
			tg.OnLMBPressed(mousePos);
		else if (input->IsButtonHeld(KeyCode::MouseLeftButton))
			tg.OnLMBHeld(mousePos);
		else if (input->IsButtonReleased(KeyCode::MouseLeftButton))
			tg.OnLMBReleased();
	}
}
