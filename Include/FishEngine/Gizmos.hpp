#pragma once

#include "ClassDef.hpp"
#include "Math.hpp"
#include "Shader.hpp"
#include "Assets.hpp"
#include <vector>


template<class T>
size_t Sizeof(const std::vector<T> v)
{
    return sizeof(T) * v.size();
}


class Gizmos : public Static
{
    friend class RenderSystem;
public:
    inline static Vector4 color;
    inline static Matrix4x4 matrix;

    
    static void StaticInit()
    {
		{
			auto vs = FISHENGINE_ROOT "Shaders/runtime/color_vs.bin";
			auto fs = FISHENGINE_ROOT "Shaders/runtime/color_fs.bin";
			auto shader = ShaderUtil::Compile(vs, fs);
			material_color = new Material;
			material_color->SetShader(shader);
		}
		{
			auto vs = FISHENGINE_ROOT "Shaders/runtime/VertexColor_vs.bin";
			auto fs = FISHENGINE_ROOT "Shaders/runtime/VertexColor_fs.bin";
			auto shader = ShaderUtil::Compile(vs, fs);
			material_vertex_color = new Material;
			material_vertex_color->SetShader(shader);
		}

        VertexPC temp;
        vb = bgfx::createDynamicVertexBuffer(bgfx::copy(&temp, sizeof(temp)), PUNTVertex::s_PC_decl, BGFX_BUFFER_ALLOW_RESIZE);
    }
    
    static void DrawCube(const Vector3& center, const Vector3& size)
    {
        auto m = Matrix4x4::Translate(center) * matrix * Matrix4x4::Scale(size);
        material_color->SetVector("u_color", color);
        Graphics::DrawMesh(Mesh::Cube, m, material_color);
    }
    

    static void DrawLine(Vector3 from, Vector3 to)
    {
		from = matrix.MultiplyPoint(from);
		to = matrix.MultiplyPoint(to);
		lines.push_back({from, color});
		lines.push_back({to, color});
    }

    static void DrawWireSphere(const Vector3& center, float radius) { }
    
private:
	
	// call by RenderSystem
	static void __Draw()
	{
		if (!lines.empty())
		{
			bgfx::update(vb, 0, bgfx::copy(lines.data(), Sizeof(lines)));
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_PT_LINES);
			material_vertex_color->BindUniforms();
			bgfx::setTransform(Matrix4x4::identity.data());
			bgfx::setVertexBuffer(0, vb);
			bgfx::submit(0, material_vertex_color->GetShader()->GetProgram());
			lines.clear();
		}
	}

	
	inline static Material* material_color = nullptr;
	inline static Material* material_vertex_color = nullptr;
    inline static std::vector<VertexPC> lines;
	inline static bgfx::DynamicVertexBufferHandle vb;
};
