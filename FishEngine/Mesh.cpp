#include "Mesh.hpp"

#include <sstream>
#include <fstream>
#include <cassert>
#include <filesystem>
namespace fs = std::experimental::filesystem;

std::string ReadFileAsString(const std::string &path)
{
	assert(fs::exists(path));
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
	
	Cube = MeshUtil::FromTextFile(ReadFileAsString(R"(D:\program\FishEngine-Experiment\Assets\Models\Cube.txt)"));

	Sphere = MeshUtil::FromTextFile(ReadFileAsString(R"(D:\program\FishEngine-Experiment\Assets\Models\Sphere.txt)"));
}

Mesh* MeshUtil::FromTextFile(const String & str)
{
	std::istringstream is(str);
	auto mesh = new Mesh();
	is >> mesh->m_vertexCount >> mesh->m_triangleCount;
	//mesh->m_vertices.resize(mesh->m_vertexCount);
	//mesh->m_normals.resize(mesh->m_vertexCount);
	//mesh->m_uv.resize(mesh->m_vertexCount);
	//mesh->m_tangents.resize(mesh->m_vertexCount);
	//mesh->m_triangles.resize(mesh->m_triangleCount * 3);
	mesh->vertices.resize(mesh->m_vertexCount);
	mesh->indices.resize(mesh->m_triangleCount * 3);


	//float vx, vy, vz;
	//Vector3 vmin(Mathf::Infinity, Mathf::Infinity, Mathf::Infinity);
	//Vector3 vmax(Mathf::NegativeInfinity, Mathf::NegativeInfinity, Mathf::NegativeInfinity);
	//for (uint32_t i = 0; i < mesh->m_vertexCount; ++i)
	//{
	//	is >> vx >> vy >> vz;
	//	if (vmin.x > vx) vmin.x = vx;
	//	if (vmin.y > vy) vmin.y = vy;
	//	if (vmin.z > vz) vmin.z = vz;
	//	if (vmax.x < vx) vmax.x = vx;
	//	if (vmax.y < vy) vmax.y = vy;
	//	if (vmax.z < vz) vmax.z = vz;
	//	auto & v = mesh->m_vertices[i];
	//	v.x = vx;
	//	v.y = vy;
	//	v.z = vz;
	//}
	//mesh->m_bounds.SetMinMax(vmin, vmax);
	//for (auto & f : mesh->m_normals)
	//	is >> f.x >> f.y >> f.z;
	//for (auto & f : mesh->m_uv)
	//	is >> f.x >> f.y;
	//for (auto & f : mesh->m_tangents)
	//	is >> f.x >> f.y >> f.z;
	//for (auto & f : mesh->m_triangles)
	//	is >> f;
	for (auto & v : mesh->vertices)
		is >> v.position.x >> v.position.y >> v.position.z;
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
