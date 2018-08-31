#include <FishEngine/Mesh.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
//#include <filesystem>
//namespace fs = std::experimental::filesystem;

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
	
	Cube = MeshUtil::FromTextFile(ReadFileAsString(R"(D:\program\FishEngine-Experiment\Assets\Models\Cube.txt)"));
	Sphere = MeshUtil::FromTextFile(ReadFileAsString(R"(D:\program\FishEngine-Experiment\Assets\Models\Sphere.txt)"));

	//const char* test_path = R"(C:\Users\yuyunkang\AppData\Local\Temp\DefaultCompany\Unity\Unity2Skfb\Unity2Skfb.gltf)";
	//const char* test_path = R"(D:\program\FishEngine-ECS\Assets\T-Rex.glb)";
	//Sphere = MeshUtil::FromGLTF(test_path);
}


void Mesh::Bind()
{
	bgfx::setVertexBuffer(0, m_VertexBuffer);
	bgfx::setIndexBuffer(m_IndexBuffer);
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

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

using tinygltf::Model;

void GetVector3(
	Vector3& v,
	int idx,
	const tinygltf::Buffer& buffer,
	const tinygltf::BufferView& bufferView,
	const tinygltf::Accessor& accessor
)
{
	int offset = accessor.byteOffset + bufferView.byteOffset;
	int stride = bufferView.byteStride == 0 ? sizeof(Vector3) : bufferView.byteStride;
	offset += stride * idx;
	auto ptr = buffer.data.data() + offset;
	memcpy(&v, ptr, sizeof(Vector3));
}

template<class T, class B>
bool In(const std::map<T, B>& d, const T& key)
{
	return d.find(key) != d.end();
}


Mesh* MeshUtil::FromGLTF(const char* filePath)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath); // for binary glTF(.glb) 

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
		abort();
	}

	Mesh* mesh = new Mesh();
	auto& primitive = model.meshes[0].primitives[0];

	bool withTangent = false;
	for (auto& pair : primitive.attributes)
	{
		if (pair.first == "TANGENT")
			withTangent = true;
	}

	int id = primitive.attributes["POSITION"];
	auto& position_accessor	= model.accessors[id];
	auto& position_bufferView = model.bufferViews[position_accessor.bufferView];
	auto& position_buffer = model.buffers[position_bufferView.buffer];


	id = primitive.attributes["NORMAL"];
	auto& normal_accessor	= model.accessors[id];
	auto& normal_bufferView = model.bufferViews[normal_accessor.bufferView];
	auto& normal_buffer = model.buffers[normal_bufferView.buffer];



	auto& indices_accessor = model.accessors[primitive.indices];
	auto& indices_bufferView = model.bufferViews[indices_accessor.bufferView];
	auto& indices_buffer = model.buffers[indices_bufferView.buffer];

	mesh->m_vertexCount = position_accessor.count;
	mesh->m_triangleCount = indices_accessor.count / 3;

	mesh->vertices.resize(mesh->m_vertexCount);
	mesh->indices.resize(mesh->m_triangleCount * 3);

	for (int i = 0; i < mesh->m_vertexCount; ++i)
	{
		auto& v = mesh->vertices[i];
		GetVector3(v.position, i, position_buffer, position_bufferView, position_accessor);
		GetVector3(v.normal, i, normal_buffer, normal_bufferView, normal_accessor);
		//GetVector3(v.tangent, i, tangent_buffer, tangent_bufferView, tangent_accessor);
		v.position *= 0.01f;
	}

	if (withTangent)
	{
		id = primitive.attributes["TANGENT"];
		auto& tangent_accessor = model.accessors[id];
		auto& tangent_bufferView = model.bufferViews[tangent_accessor.bufferView];
		auto& tangent_buffer = model.buffers[tangent_bufferView.buffer];

		for (int i = 0; i < mesh->m_vertexCount; ++i)
		{
			auto& v = mesh->vertices[i];
			GetVector3(v.tangent, i, tangent_buffer, tangent_bufferView, tangent_accessor);
		}
	}

	//assert(mesh->indices.size() * sizeof())
	//memcpy(mesh->indices.data(), ptr, indices_bufferView.byteLength);

	mesh->m_VertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(mesh->vertices.data(), sizeof(PUNTVertex)*mesh->vertices.size()),
		PUNTVertex::ms_decl
	);

	//5120 (BYTE)1
	//5121(UNSIGNED_BYTE)1
	//5122 (SHORT)2
	//5123 (UNSIGNED_SHORT)2
	//5125 (UNSIGNED_INT)4
	//5126 (FLOAT)4
	int size = 1;	// in bytes
	switch (indices_accessor.componentType)
	{
	case 5120: // byte
	case 5121:
		size = 1; break;
	case 5122:
	case 5123:
		size = 2; break;
	case 5125:
	case 5126:
		size = 4; break;
	}

	auto ptr = indices_buffer.data.data();
	ptr += indices_bufferView.byteOffset + indices_accessor.byteOffset;

	assert(indices_bufferView.byteLength == size * indices_accessor.count);
	assert(size == 2);

	//if (indices_accessor.componentType == 5123)
	//{
	//	memcpy(mesh->indices.data(), ptr, indices_bufferView.byteLength);
	//	//printf("%s\n", mesh->indices[0]);

	//	mesh->m_IndexBuffer = bgfx::createIndexBuffer(
	//		bgfx::makeRef(mesh->indices.data(), sizeof(uint16_t)*mesh->indices.size())
	//	);
	//}
	//else
	//{
	//	mesh->m_IndexBuffer = bgfx::createIndexBuffer(
	//		bgfx::copy(ptr, indices_bufferView.byteLength)
	//	);
	//}

	mesh->m_IndexBuffer = bgfx::createIndexBuffer(
		bgfx::copy(ptr, indices_bufferView.byteLength)
	);

	return mesh;
}
