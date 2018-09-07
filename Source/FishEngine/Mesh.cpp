#include <FishEngine/Mesh.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
#include <FishEngine/Assets.hpp>
//#include <filesystem>
//namespace fs = std::experimental::filesystem;

#include <FishEngine/Texture.hpp>

std::string ReadFileAsString(const std::string &path)
{
//	assert(fs::exists(path));
	std::ifstream is(path);
	std::stringstream buffer;
	buffer << is.rdbuf();
	return buffer.str();
}


void PUNTVertex::StaticInit()
{
	ms_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Float)
		.end();

	s_P_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
}

bgfx::VertexDecl PUNTVertex::ms_decl;
bgfx::VertexDecl PUNTVertex::s_P_decl;

void Mesh::StaticInit()
{
	PUNTVertex::StaticInit();
	
	Cube = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Cube.txt"));
	Sphere = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Sphere.txt"));
}


void Mesh::Bind(int subMeshIndex/* = -1*/, int bgfxStream/* = 0*/)
{
	if (subMeshIndex < -1 || subMeshIndex >= m_SubMeshCount)
	{
		return;
	}

	if (IsSkinned())
		bgfx::setVertexBuffer(bgfxStream, m_DynamicVertexBuffer);
	else
		bgfx::setVertexBuffer(bgfxStream, m_VertexBuffer);

	if (subMeshIndex == -1)
		bgfx::setIndexBuffer(m_IndexBuffer);
	else
	{
		auto& info = m_SubMeshInfos[subMeshIndex];
		bgfx::setIndexBuffer(m_IndexBuffer, info.StartIndex, info.Length);
	}
}

Mesh* MeshUtil::FromTextFile(const String & str)
{
	std::istringstream is(str);
	auto mesh = new Mesh();
	is >> mesh->m_VertexCount >> mesh->m_TriangleCount;

	mesh->m_Vertices.resize(mesh->m_VertexCount);
	mesh->m_Indices.resize(mesh->m_TriangleCount * 3);

	for (auto & v : mesh->m_Vertices)
	{
		is >> v.position.x >> v.position.y >> v.position.z;
	}
	for (auto & v : mesh->m_Vertices)
		is >> v.normal.x >> v.normal.y >> v.normal.z;
	for (auto & v : mesh->m_Vertices)
		is >> v.uv.x >> v.uv.y;
	for (auto & v : mesh->m_Vertices)
		is >> v.tangent.x >> v.tangent.y >> v.tangent.z;
	for (auto & f : mesh->m_Indices)
		is >> f;

	mesh->m_VertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(mesh->m_Vertices.data(), sizeof(PUNTVertex)*mesh->m_Vertices.size()),
		PUNTVertex::ms_decl
	);

	mesh->m_IndexBuffer = bgfx::createIndexBuffer(
		bgfx::makeRef(mesh->m_Indices.data(), sizeof(uint16_t)*mesh->m_Indices.size())
	);

	return mesh;
}
