#pragma once

#include "ClassDef.hpp"
#include "Math.hpp"
#include "Material.hpp"
#include "MeshVertex.hpp"
#include <vector>


class Gizmos : public Static
{
    friend class RenderSystem;
public:
    inline static Vector4 color;
    inline static Matrix4x4 matrix;

    static void StaticInit();
    static void DrawCube(const Vector3& center, const Vector3& size);
    static void DrawLine(Vector3 from, Vector3 to);
	static void DrawRay(Vector3 from, const Vector3& direction);
    static void DrawWireSphere(const Vector3& center, float radius);
	static void DrawBounds(const Bounds& bounds);
	static void DrawFrustum(const Frustum& frustum, const Matrix4x4& cameraToWorld);
    
private:
	
	// call by RenderSystem
	static void __Draw();

	
	inline static Material* s_ColorMaterial = nullptr;
	inline static Material* s_VertexColorMaterial = nullptr;
    inline static std::vector<VertexPC> s_Lines;
	inline static bgfx::DynamicVertexBufferHandle s_LineDynamicVertexBuffer;

public:
	static bool s_EnableDepthTest;
};
