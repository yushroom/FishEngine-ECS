#pragma once

#include "Engine.hpp"
#include <bgfx/bgfx.h>
#include "Object.hpp"
#include "Math.hpp"

struct PUNTVertex
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;

	static void init();;

	static bgfx::VertexDecl ms_decl;
};


class Mesh : public Object
{
	friend class MeshUtil;
public:
	bgfx::IndexBufferHandle m_IndexBuffer;
	bgfx::VertexBufferHandle m_VertexBuffer;
	
	inline static Mesh* Cube = nullptr;
	inline static Mesh* Sphere = nullptr;
	inline static Mesh* Bunny = nullptr;
	
	static void StaticInit();

	void Bind();

private:
	uint32_t m_vertexCount = 0;
	uint32_t m_triangleCount = 0;
	int						m_subMeshCount = 1;
	//std::vector<Vector3>    m_vertices;
	//std::vector<Vector3>    m_normals;
	//std::vector<Vector2>    m_uv;
	//std::vector<Vector3>    m_tangents;
	//std::vector<uint32_t>   m_triangles;
	//std::vector<uint32_t>	m_subMeshIndexOffset;	// index start

	std::vector<PUNTVertex> vertices;
	std::vector<uint16_t> indices;
};


class MeshUtil : public Static
{
public:
	static Mesh* FromTextFile(const String & str);
	static Mesh* FromGLTF(const char* filePath);
};