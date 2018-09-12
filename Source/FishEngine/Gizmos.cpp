#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Assets.hpp>
#include <FishEngine/Graphics.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Mesh.hpp>

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
	s_LineDynamicVertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::copy(&temp, sizeof(temp)), PUNTVertex::s_PC_decl, BGFX_BUFFER_ALLOW_RESIZE);
}

void Gizmos::DrawCube(const Vector3& center, const Vector3& size)
{
	auto m = Matrix4x4::Translate(center) * matrix * Matrix4x4::Scale(size);
	s_ColorMaterial->SetVector("u_color", color);
	Graphics::DrawMesh(Mesh::Cube, m, s_ColorMaterial);
}

void Gizmos::DrawLine(Vector3 from, Vector3 to)
{
	from = matrix.MultiplyPoint(from);
	to = matrix.MultiplyPoint(to);
	s_Lines.push_back({ from, color });
	s_Lines.push_back({ to, color });
}

void Gizmos::DrawWireSphere(const Vector3& center, float radius)
{
	abort();
}

void Gizmos::__Draw()
{
	if (!s_Lines.empty())
	{
		bgfx::update(s_LineDynamicVertexBuffer, 0, bgfx::copy(s_Lines.data(), Sizeof(s_Lines)));
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_PT_LINES);
		s_VertexColorMaterial->BindUniforms();
		bgfx::setTransform(Matrix4x4::identity.data());
		bgfx::setVertexBuffer(0, s_LineDynamicVertexBuffer, 0, s_Lines.size());
		bgfx::submit(0, s_VertexColorMaterial->GetShader()->GetProgram());
		s_Lines.clear();
	}
}
