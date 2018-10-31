#include <FishEngine/Mesh.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
#include <FishEngine/Assets.hpp>
//#include <filesystem>
//namespace fs = std::experimental::filesystem;

#include <FishEngine/Model.hpp>

#include <FishEngine/Texture.hpp>
#include <FishEngine/bgfxHelper.hpp>

using namespace FishEngine;

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
//	ms_decl
//		.begin()
//		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Float)
//		.end();
//	assert(PUNTVertex::ms_decl.getStride() == sizeof(PUNTVertex));
	
	ms_decl.Begin()
		.Add(VertexAttrib::Position, 3, VertexAttribType::Float)
		.Add(VertexAttrib::TexCoord0, 2, VertexAttribType::Float)
		.Add(VertexAttrib::Normal, 3, VertexAttribType::Float)
		.Add(VertexAttrib::Tangernt, 4, VertexAttribType::Float)
	.End();
	assert(ms_decl.GetStride() == sizeof(PUNTVertex));

//	s_P_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
//
//	s_PC_decl.begin()
//		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
//	.end();
}

//bgfx::VertexDecl PUNTVertex::ms_decl;
//bgfx::VertexDecl PUNTVertex::s_P_decl;
VertexDecl PUNTVertex::ms_decl;

void Mesh::StaticInit()
{
//	PUNTVertex::StaticInit();

//	auto meshes = ModelUtil::LoadMeshesFromGLTF(FISHENGINE_ROOT "Assets/Models/Unity2Skfb.gltf");
//	assert(meshes.size() == 6);

//	Quad = meshes[0];
//	Cube = meshes[1];
//	Capsule = meshes[2];
//	Cylinder = meshes[3];
//	Sphere = meshes[4];
//	Plane = meshes[5];

	Cube = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Cube.txt"));
	Sphere = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Sphere.txt"));
	Cylinder = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Cylinder.txt"));
	Cone = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Cone.txt"));
	Plane = MeshUtil::FromTextFile(ReadFileAsString(FISHENGINE_ROOT "Assets/Models/Plane.txt"));
}

void Mesh::__Upload()
{
//	m_VertexBuffer = bgfx::createVertexBuffer(bgfxHelper::MakeRef(m_Vertices), PUNTVertex::ms_decl);
//	m_IndexBuffer = bgfx::createIndexBuffer(bgfxHelper::MakeRef(m_Indices), BGFX_BUFFER_INDEX32);
	
//	VertexDecl decl;
//	decl.SetVertexSize(sizeof(PUNTVertex));
	m_VertexBuffer = CreateVertexBuffer(Memory::FromVectorArray(m_Vertices), PUNTVertex::ms_decl);
	m_IndexBuffer = CreateIndexBuffer(Memory::FromVectorArray(m_Indices), MeshIndexType::UInt32);
}

void Mesh::Bind(int subMeshIndex/* = -1*/, int bgfxStream/* = 0*/)
{
	if (subMeshIndex < -1 || subMeshIndex >= m_SubMeshCount)
	{
		abort();
		return;
	}

//	if (IsSkinned())
//		bgfx::setVertexBuffer(bgfxStream, m_DynamicVertexBuffer);
//	else
//		bgfx::setVertexBuffer(bgfxStream, m_VertexBuffer);
//
//	if (subMeshIndex == -1)
//		bgfx::setIndexBuffer(m_IndexBuffer);
//	else
//	{
//		auto& info = m_SubMeshInfos[subMeshIndex];
//		bgfx::setIndexBuffer(m_IndexBuffer, info.StartIndex, info.Length);
//	}
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
	
	mesh->m_SubMeshCount = 1;
	SubMeshInfo info;
	info.StartIndex = 0;
	info.VertexOffset = 0;
	info.Length = (int)mesh->m_Indices.size();
	mesh->m_SubMeshInfos.push_back(info);
	mesh->__Upload();

	return mesh;
}
