#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Assets.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Mesh.hpp>
#include <cassert>
#include <FishEngine/bgfxHelper.hpp>
#include <FishEngine/Render/RenderViewType.hpp>

using namespace FishEngine;

void Gizmos::StaticInit()
{
	s_ColorMaterial = Material::Clone(Material::ColorMaterial);
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/VertexColor_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/VertexColor_fs.bin";
		auto shader = ShaderUtil::Compile(vs, fs);
		s_VertexColorMaterial = new Material;
		s_VertexColorMaterial->SetShader(shader);
	}

	VertexPC temp;
//	s_LineDynamicVertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::copy(&temp, sizeof(temp)), PUNTVertex::s_PC_decl, BGFX_BUFFER_ALLOW_RESIZE);

	{
		for (int i = 0; i < circle_vertex_count-1; ++i)
		{
			float theta = 2*Mathf::PI / (circle_vertex_count-1) * i;
			vertices[i].Set(sin(theta), cos(theta), 0);
		}
		vertices[circle_vertex_count-1] = vertices[0];
//		s_CircleVertexBuffer = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(Vector3)*circle_vertex_count), PUNTVertex::s_P_decl);
	}
}

void Gizmos::DrawCube(const Vector3& center, const Vector3& size)
{
	auto m = Matrix4x4::Translate(center) * Matrix4x4::Scale(size);
	s_ColorMaterial->SetVector("u_color", color);
	Graphics::DrawMesh(Mesh::Cube, m, s_ColorMaterial);
}

void Gizmos::DrawLine(Vector3 from, Vector3 to)
{
	//Vector3 t, s;
	//Quaternion r;
	//Matrix4x4::Decompose(matrix, &t, &r, &s);
	//auto mat = Matrix4x4::TRS(t, r, Vector3::one);
	auto& mat = matrix;
	from = mat.MultiplyPoint(from);
	to = mat.MultiplyPoint(to);
	s_Lines.push_back({ from, color });
	s_Lines.push_back({ to, color });
}

void Gizmos::DrawRay(Vector3 from, const Vector3& direction)
{
	DrawLine(from, from + direction);
}

void Gizmos::DrawWireSphere(const Vector3& center, float radius)
{
	abort();
}

void Gizmos::DrawBounds(const Bounds& bounds)
{
	if (!bounds.IsValid())
	{
		printf("[%s:%d -- %s] invalid bounds\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	const static Vector3 lines[] = {
		{ 1, 1, 1 },{ 1, 1, -1 },
		{ 1, 1, 1 },{ 1, -1, 1 },
		{ 1, 1, 1 },{ -1, 1, 1 },
		{ -1, -1, -1 },{ -1, -1, 1 },
		{ -1, -1, -1 },{ -1, 1, -1 },
		{ -1, -1, -1 },{ 1, -1, -1 },
		{ -1, -1, 1 },{ -1, 1, 1 },
		{ -1, -1, 1 },{ 1, -1, 1 },
		{ -1, 1, -1 },{ -1, 1, 1 },
		{ -1, 1, -1 },{ 1, 1, -1 },
		{ 1, -1, -1 },{ 1, -1, 1 },
		{ 1, -1, -1 },{ 1, 1, -1 }
	};

	//auto mat = matrix;
	auto mat = matrix;
//	auto ext = bounds.extents();
	//matrix = Matrix4x4::Translate(bounds.center()) * Matrix4x4::Scale(bounds.extents()) * matrix;
	matrix =  matrix * Matrix4x4::Translate(bounds.center()) * Matrix4x4::Scale(bounds.extents());
	for (int i = 0; i < 12; ++i)
	{
		DrawLine(lines[i * 2], lines[i * 2 + 1]);
	}
	matrix = mat;
}


void Gizmos::DrawFrustum(const Frustum& frustum, const Matrix4x4& cameraToWorld)
{
	constexpr int numLines = 4 * 3;
	static const int indices[numLines * 2] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	Vector3 corners[8];
	frustum.getLocalCorners(corners);
	for (auto& c : corners)
	{
		c = cameraToWorld.MultiplyPoint(c);
	}

	auto mat = matrix;
	matrix = Matrix4x4::identity;
	for (int i = 0; i < numLines*2; i += 2)
	{
		DrawLine(corners[indices[i]], corners[indices[i+1]]);
	}
	matrix = mat;
}


void Gizmos::DrawCircle(const Vector3& center, float radius)
{
#if 0
	uint64_t state = BGFX_STATE_WRITE_RGB
		| BGFX_STATE_DEPTH_TEST_ALWAYS
		| BGFX_STATE_PT_LINESTRIP;
	Matrix4x4 m = Gizmos::matrix * Matrix4x4::Translate(center) *  Matrix4x4::Scale(radius);
	bgfx::setTransform(m.transpose().data());
	bgfx::setState(state);
	bgfx::setVertexBuffer(0, Gizmos::s_CircleVertexBuffer);
	s_ColorMaterial->SetVector("u_color", Gizmos::color);
	s_ColorMaterial->BindUniforms();
	bgfx::submit(0, s_ColorMaterial->GetShader()->GetProgram());
//	for (auto v : vertices)
//	float d = m.MultiplyPoint(center).z + 0.1f;
//	matrix = Matrix4x4::identity;
//	for (int i = 0; i < circle_vertex_count-1; ++i)
//	{
//		auto v1 = vertices[i];
//		v1 = m.MultiplyPoint(v1);
//		auto v2 = vertices[i+1];
//		v2 = m.MultiplyPoint(v2);
//		if (v1.z < d && v2.z < d)
//		{
//			DrawLine(v1, v2);
//		}
//	}
#endif
}


void Gizmos::__Draw()
{
#if 0
	if (!s_Lines.empty())
	{
		bgfx::update(s_LineDynamicVertexBuffer, 0, bgfxHelper::MakeCopy(s_Lines));
		uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_PT_LINES;
		if (s_EnableDepthTest)
			state |= BGFX_STATE_DEPTH_TEST_LESS;
		else
			state |= BGFX_STATE_DEPTH_TEST_ALWAYS;
//		state |= BGFX_STATE_LINEAA;
		state |= BGFX_STATE_MSAA;
		bgfx::setState(state);
		s_VertexColorMaterial->BindUniforms();
		bgfx::setTransform(Matrix4x4::identity.data());
		auto viewId = (bgfx::ViewId)RenderViewType::Scene;
		bgfx::setVertexBuffer(viewId, s_LineDynamicVertexBuffer, 0, (uint32_t)s_Lines.size());
		bgfx::submit(viewId, s_VertexColorMaterial->GetShader()->GetProgram());
		s_Lines.clear();
	}
#endif
}


bool Gizmos::s_EnableDepthTest = false;
