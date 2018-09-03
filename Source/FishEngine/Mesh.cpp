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
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

//using tinygltf::Model;

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

void LoadBuffer(const tinygltf::Model& model, int accessorID, std::vector<float>& out_buffer)
{
	auto& accessor = model.accessors[accessorID];
	auto& bufferView = model.bufferViews[accessor.bufferView];
	auto& buffer = model.buffers[bufferView.buffer];
	assert(accessor.componentType == 5126);
	int s = 1;
	if (accessor.type == TINYGLTF_TYPE_SCALAR)
		s = 1;
	else if (accessor.type == TINYGLTF_TYPE_VEC3)
		s = 3;
	else if (accessor.type == TINYGLTF_TYPE_VEC4)
		s = 4;
	else
		abort();
	out_buffer.resize(accessor.count * s);
	
	int offset = accessor.byteOffset + bufferView.byteOffset;
	auto ptr = buffer.data.data() + offset;
	memcpy(out_buffer.data(), ptr, accessor.count * s * sizeof(float));
}


void ImportMesh(Mesh* mesh, const tinygltf::Model& model, tinygltf::Mesh& gltf_mesh)
{
	assert(gltf_mesh.primitives.size() == 1);
	auto& primitive = gltf_mesh.primitives[0];
	
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
	
	mesh->m_VertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(mesh->vertices.data(),
																  sizeof(PUNTVertex)*mesh->vertices.size()),
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
	auto byteLen = size * indices_accessor.count;
	
	assert(indices_bufferView.byteLength >= size * indices_accessor.count);
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
	
	mesh->m_IndexBuffer = bgfx::createIndexBuffer( bgfx::copy(ptr, byteLen) );
}

#include <FishEngine/Components/Animator.hpp>

void ImportAnimator(Animator* animator, const tinygltf::Model& model, const std::vector<ECS::GameObject*>& gos)
{
	auto& anim = model.animations[0];
	int channelCount = anim.channels.size();
//	Animator* animator = new Animator();
	animator->curves.resize(channelCount);
	for (int i = 0; i < channelCount; ++i)
	{
		auto& curve = animator->curves[i];
		auto& channel = anim.channels[i];
		curve.node = gos[channel.target_node];
		if (channel.target_path == "translation")
			curve.type = AnimationCurveType::Translation;
		else if (channel.target_path == "rotation")
			curve.type = AnimationCurveType::Rotation;
		else if (channel.target_path == "scale")
			curve.type = AnimationCurveType::Scale;
		else if (channel.target_path == "weights")
			curve.type = AnimationCurveType::Weights;
		else
			abort();
		
		auto& sampler = anim.samplers[channel.sampler];
		
		{
//			auto& accessor = model.accessors[sampler.input];
//			auto& bufferView = model.bufferViews[accessor.bufferView];
//			auto& buffer = model.buffers[bufferView.buffer];
//			curve.input.resize(accessor.count);
//			assert(accessor.componentType == 5126);
//			assert(accessor.type == TINYGLTF_TYPE_SCALAR);
//
//			int offset = accessor.byteOffset + bufferView.byteOffset;
//			auto ptr = buffer.data.data() + offset;
//			memcpy(curve.input.data(), ptr, accessor.count*sizeof(float));
			LoadBuffer(model, sampler.input, curve.input);
		}
		
		{
//			auto& accessor = model.accessors[sampler.input];
//			auto& bufferView = model.bufferViews[accessor.bufferView];
//			auto& buffer = model.buffers[bufferView.buffer];
//			curve.input.resize(accessor.count);
//			assert(accessor.componentType == 5126);
//			if (curve.type == AnimationCurveType::Rotation)
//				assert(accessor.type == TINYGLTF_TYPE_VEC4);
//			else
//				abort();
//
//
//			int offset = accessor.byteOffset + bufferView.byteOffset;
//			auto ptr = buffer.data.data() + offset;
//			memcpy(curve.input.data(), ptr, accessor.count*sizeof(float));
			LoadBuffer(model, sampler.output, curve.output);
		}
		
		//assert(curve.input.size() == curve.output.size());
	}
}


tinygltf::Model ModelUtil::FromGLTF(const char* filePath, ECS::Scene* scene)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

//	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
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

	std::vector<Mesh*> meshes;
	for (int i = 0; i < model.meshes.size(); ++i)
	{
		Mesh* mesh = new Mesh();
		ImportMesh(mesh, model, model.meshes[i]);
		meshes.push_back(mesh);
	}
	
	std::vector<ECS::GameObject*> nodes;
	nodes.reserve(model.nodes.size());
	for (auto& node : model.nodes)
	{
		nodes.push_back(scene->CreateGameObject());
		if (node.mesh != -1)
		{
//			scene->GameObjectAddComponent<>(<#ECS::GameObject *go#>)
		}
	}
	
	Animator* animator = new Animator();
	ImportAnimator(animator, model, nodes);
	
//	auto& img = model.images[0].image;
////	auto tex = TextureUtils::LoadTextureFromMemory(img.data(), img.size());
//	auto tex = loadTexture2(img.data(), img.size(), "from/gltf", BGFX_TEXTURE_NONE, nullptr, nullptr);

	int nodeId = model.scenes[model.defaultScene].nodes[0];
	return model;
}
