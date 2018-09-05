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


void PUNTVertex::init()
{
	ms_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
		.end();
}

bgfx::VertexDecl PUNTVertex::ms_decl;


void Mesh::StaticInit()
{
	PUNTVertex::init();
	
	Cube = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Cube.txt"));
	Sphere = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Sphere.txt"));
}


void Mesh::Bind()
{
	if (IsSkinned())
		bgfx::setVertexBuffer(0, m_DynamicVertexBuffer);
	else
		bgfx::setVertexBuffer(0, m_VertexBuffer);
	bgfx::setIndexBuffer(m_IndexBuffer);
}

Mesh* MeshUtil::FromTextFile(const String & str)
{
	std::istringstream is(str);
	auto mesh = new Mesh();
	is >> mesh->m_vertexCount >> mesh->m_triangleCount;

	mesh->vertices.resize(mesh->m_vertexCount);
	mesh->indices.resize(mesh->m_triangleCount * 3);

	for (auto & v : mesh->vertices)
	{
		is >> v.position.x >> v.position.y >> v.position.z;
		//v.position.x *= -1;
		v.position /= 20;
	}
	for (auto & v : mesh->vertices)
		is >> v.normal.x >> v.normal.y >> v.normal.z;
	for (auto & v : mesh->vertices)
		is >> v.uv.x >> v.uv.y;
	for (auto & v : mesh->vertices)
		is >> v.tangent.x >> v.tangent.y >> v.tangent.z;
	for (auto & f : mesh->indices)
		is >> f;

	mesh->m_VertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(mesh->vertices.data(), sizeof(PUNTVertex)*mesh->vertices.size()),
		PUNTVertex::ms_decl
	);

	mesh->m_IndexBuffer = bgfx::createIndexBuffer(
		bgfx::makeRef(mesh->indices.data(), sizeof(uint16_t)*mesh->indices.size())
	);

	return mesh;
}
