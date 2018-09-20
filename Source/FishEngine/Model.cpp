#include <FishEngine/Model.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/ECS/GameObject.hpp>
#include <FishEngine/ECS/Scene.hpp>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

// gltf uses right handed coordinate system(rhs)
// set this flag to converted to left handed system(lhs)
#define LOAD_GLTF_AS_LHS 0

using namespace FishEngine;

struct Model
{
	GLTFLoadFlags flags;
	GameObject* rootGameObject = nullptr;
	std::vector<GameObject*> nodes;
	std::vector<Mesh*> meshes;
	std::vector<Skin*> skins;
	std::vector<bgfx::TextureHandle> images;
	std::vector<Material*> materials;
	std::vector<AnimationClip*> animations;
	//tinygltf::Model gltfModel;
};


void GetVector2(
	Vector2& v,
	int idx,
	const tinygltf::Buffer& buffer,
	const tinygltf::BufferView& bufferView,
	const tinygltf::Accessor& accessor
)
{
	assert(accessor.type == TINYGLTF_TYPE_VEC2);
	assert(accessor.componentType == 5126);
	assert(idx < accessor.count);
	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	size_t stride = bufferView.byteStride == 0 ? sizeof(Vector2) : bufferView.byteStride;
	offset += stride * idx;
	//assert(offset < bufferView.byteLength);
	auto ptr = buffer.data.data() + offset;
	memcpy(&v, ptr, sizeof(Vector2));
}

void GetVector3(
	Vector3& v,
	int idx,
	const tinygltf::Buffer& buffer,
	const tinygltf::BufferView& bufferView,
	const tinygltf::Accessor& accessor
)
{
	assert(accessor.type == TINYGLTF_TYPE_VEC3);
	assert(accessor.componentType == 5126);
	assert(idx < accessor.count);
	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	size_t stride = bufferView.byteStride == 0 ? sizeof(Vector3) : bufferView.byteStride;
	offset += stride * idx;
	//assert(offset < bufferView.byteLength);
	auto ptr = buffer.data.data() + offset;
	memcpy(&v, ptr, sizeof(Vector3));
}

void GetVector4(
	Vector4& v,
	int idx,
	const tinygltf::Buffer& buffer,
	const tinygltf::BufferView& bufferView,
	const tinygltf::Accessor& accessor
)
{
	assert(accessor.type == TINYGLTF_TYPE_VEC4);
	assert(accessor.componentType == 5126);
	assert(idx < accessor.count);
	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	size_t stride = bufferView.byteStride == 0 ? sizeof(Vector4) : bufferView.byteStride;
	offset += stride * idx;
	//assert(offset < bufferView.byteLength);
	auto ptr = buffer.data.data() + offset;
	memcpy(&v, ptr, sizeof(Vector4));
}


template<class T, class B>
bool In(const std::map<T, B>& d, const char* key)
{
	return d.find(key) != d.end();
}

template<class T, class B>
const B& Get(const std::map<T, B>& dict, const char* key)
{
	assert(dict.find(key) != dict.end());
	return dict.find(key)->second;
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

	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	auto ptr = buffer.data.data() + offset;
	memcpy(out_buffer.data(), ptr, accessor.count * s * sizeof(float));
}


#if LOAD_GLTF_AS_LHS

inline void RHS2LHS(Vector3& v)
{
	v.z *= -1;
}

inline void RHS2LHS(Quaternion& q)
{
	auto euler = q.eulerAngles();
	euler.z *= -1;
	q = Quaternion::Euler(euler);
}

inline void RHS2LHS(Matrix4x4& m)
{
	Vector3 t, s;
	Quaternion r;
	Matrix4x4::Decompose(m, &t, &r, &s);
	RHS2LHS(t);
	RHS2LHS(r);
	m.SetTRS(t, r, s);
}
#else
inline void RHS2LHS(Vector3& v)
{
}

inline void RHS2LHS(Quaternion& q)
{
}

inline void RHS2LHS(Matrix4x4& m)
{
}
#endif

void ImportPrimitive(Mesh* mesh,
	const tinygltf::Model& model,
	const SubMeshInfo& info,
	const tinygltf::Primitive& primitive,
	bool skinned)
{
	bool withPosition = In(primitive.attributes, "POSITION");
	bool withNormal = false;
	bool withTangent = false;
	bool withJoints = false;
	bool withWeights = false;
	bool withUV = false;
//	int positionID = -1;
//	int normalID = -1;
//	int tangentID = -1;
//	int jointsID = -1;
//	int weightsID = -1;
//	int uvID = -1;
	for (auto& pair : primitive.attributes)
	{
		auto& attr = pair.first;
		if (attr == "NORMAL")
			withNormal = true;
		else if (attr == "TANGENT")
			withTangent = true;
		else if (attr == "JOINTS_0")
			withJoints = true;
		else if (attr == "WEIGHTS_0")
			withWeights = true;
		else if (attr == "TEXCOORD_0")
			withUV = true;
	}
	
	assert(withPosition);
	
	if (skinned)
	{
		assert(withJoints);
		assert(withWeights);
	}
	else
	{
		assert(!withJoints);
		assert(!withWeights);
	}

	// TODO: primitive.mode

	int id = Get(primitive.attributes, "POSITION");
	auto& position_accessor = model.accessors[id];
	auto& position_bufferView = model.bufferViews[position_accessor.bufferView];
	auto& position_buffer = model.buffers[position_bufferView.buffer];
	
	{
		auto& m = position_accessor.minValues;
		Vector3 minv((float)m[0], (float)m[1], (float)m[2]);
		auto& m2 = position_accessor.maxValues;
		Vector3 maxv((float)m2[0], (float)m2[1], (float)m2[2]);
		//mesh->bounds.SetMinMax(minv, maxv);
		mesh->bounds.Encapsulate(minv);
		mesh->bounds.Encapsulate(maxv);
	}

	auto primitiveVertexCount = position_accessor.count;

	for (int i = 0; i < primitiveVertexCount; ++i)
	{
		auto& v = mesh->m_Vertices[i+info.VertexOffset];
		GetVector3(v.position, i, position_buffer, position_bufferView, position_accessor);
		RHS2LHS(v.position);
	}
	
	if (withNormal)
	{
		id = Get(primitive.attributes, "NORMAL");
		auto& normal_accessor = model.accessors[id];
		auto& normal_bufferView = model.bufferViews[normal_accessor.bufferView];
		auto& normal_buffer = model.buffers[normal_bufferView.buffer];
		assert(normal_accessor.count == primitiveVertexCount);
		
		for (int i = 0; i < primitiveVertexCount; ++i)
		{
			auto& v = mesh->m_Vertices[i+info.VertexOffset];
			GetVector3(v.normal, i, normal_buffer, normal_bufferView, normal_accessor);
			RHS2LHS(v.normal);
		}
	}

	if (withUV)
	{
		id = Get(primitive.attributes, "TEXCOORD_0");
		auto& accessor = model.accessors[id];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];
		assert(accessor.count == primitiveVertexCount);

		for (int i = 0; i < primitiveVertexCount; ++i)
		{
			auto& v = mesh->m_Vertices[i + info.VertexOffset];
			GetVector2(v.uv, i, buffer, bufferView, accessor);
		}
	}

	if (withTangent)
	{
		id = Get(primitive.attributes, "TANGENT");
		auto& accessor = model.accessors[id];
		auto& tangent_bufferView = model.bufferViews[accessor.bufferView];
		auto& tangent_buffer = model.buffers[tangent_bufferView.buffer];
		assert(accessor.count == primitiveVertexCount);

		for (int i = 0; i < primitiveVertexCount; ++i)
		{
			auto& v = mesh->m_Vertices[i + info.VertexOffset];
			GetVector4(v.tangent, i, tangent_buffer, tangent_bufferView, accessor);
			//TODO: RHS 2 LHS
		}
	}

	if (withJoints)
	{
		mesh->joints.resize(mesh->m_VertexCount);
		id = Get(primitive.attributes, "JOINTS_0");
		auto& accessor = model.accessors[id];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];
		assert(accessor.count == primitiveVertexCount);

		auto offset = accessor.byteOffset + bufferView.byteOffset;
		auto ptr = buffer.data.data() + offset;

		if (accessor.componentType == 5123)	// unsigned short
		{
			auto p = (unsigned short*)ptr;
			for (int i = 0; i < accessor.count; ++i)
			{
				auto& joint = mesh->joints[i+info.VertexOffset];
				joint.x = *p; ++p;
				joint.y = *p; ++p;
				joint.z = *p; ++p;
				joint.w = *p; ++p;
			}
		}
		else if (accessor.componentType == 5121)	// unsigned byte
		{
			auto p = (unsigned char*)ptr;
			for (int i = 0; i < accessor.count; ++i)
			{
				auto& joint = mesh->joints[i+info.VertexOffset];
				joint.x = *p; ++p;
				joint.y = *p; ++p;
				joint.z = *p; ++p;
				joint.w = *p; ++p;
			}
		}
		else
		{
			abort();
		}
	}

	if (withWeights)
	{
//		mesh->weights.resize(mesh->m_VertexCount);
		id = Get(primitive.attributes, "WEIGHTS_0");
		auto& accessor = model.accessors[id];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];

		assert(accessor.componentType == 5126);		// float
		assert(accessor.count == primitiveVertexCount);

		auto offset = accessor.byteOffset + bufferView.byteOffset;
		auto ptr = buffer.data.data() + offset;
		auto dst = mesh->weights.data() + info.VertexOffset;
		memcpy(dst, ptr, accessor.count * 4 * sizeof(float));
	}


	auto& indices_accessor = model.accessors[primitive.indices];
	auto& indices_bufferView = model.bufferViews[indices_accessor.bufferView];
	auto& indices_buffer = model.buffers[indices_bufferView.buffer];
	
	assert(indices_accessor.type == TINYGLTF_TYPE_SCALAR);

	int size = 1;	// in bytes
	switch (indices_accessor.componentType)
	{
//	case 5120:	// byte
	case 5121:	// unsigned byte
		size = 1; break;
//	case 5122:	// short
	case 5123:	// unsigned short
		size = 2; break;
	case 5125:	// unsigned int
//	case 5126:	// float
		size = 4; break;
	default:
		abort();
	}

	auto ptr = indices_buffer.data.data();
	auto offset = indices_bufferView.byteOffset + indices_accessor.byteOffset;
	ptr += offset;
//	auto byteLen = size * indices_accessor.count;
//	assert(indices_bufferView.byteLength == byteLen);
	
	if (size == 1)
	{
		auto p = (unsigned char*)ptr;
		for (int i = 0; i < indices_accessor.count; ++i)
		{
			mesh->m_Indices[i + info.StartIndex] = info.VertexOffset + *p;
			p++;
		}
	}
	else if (size == 2)
	{
		auto p = (unsigned short*)ptr;
		for (int i = 0; i < indices_accessor.count; ++i)
		{
			mesh->m_Indices[i + info.StartIndex] = info.VertexOffset + *p;
			p++;
		}
	}
	else
	{
		auto p = (unsigned int*)ptr;
		for (int i = 0; i < indices_accessor.count; ++i)
		{
			mesh->m_Indices[i + info.StartIndex] = info.VertexOffset + *p;
			p++;
		}
	}
}

void ImportMesh(Mesh* mesh, const tinygltf::Model& model, tinygltf::Mesh& gltf_mesh)
{
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	mesh->name = gltf_mesh.name;
	mesh->m_SubMeshCount = (int)gltf_mesh.primitives.size();
	mesh->m_SubMeshInfos.resize(mesh->m_SubMeshCount);
	bool skinned = false;
	for (int i = 0; i < mesh->m_SubMeshCount; ++i)
	{
		auto& primitive = gltf_mesh.primitives[i];
		auto& info = mesh->m_SubMeshInfos[i];
		{
			int id = primitive.attributes["POSITION"];
			auto& accessor = model.accessors[id];
//			if (vertexCount + accessor.count > std::numeric_limits<unsigned short>::max())
//			{
//				abort();
//			}
			info.VertexOffset = vertexCount;
			vertexCount += (uint32_t)accessor.count;
			
			if (In(primitive.attributes, "JOINTS_0"))
			{
				skinned = true;
			}
		}

		{
			int id = primitive.indices;
			auto& accessor = model.accessors[id];
			info.StartIndex = indexCount;
			info.Length = (int)accessor.count;
			indexCount += (uint32_t)accessor.count;
		}
	}
	mesh->m_VertexCount = vertexCount;
	mesh->m_Vertices.resize(vertexCount);
	if (skinned)
	{
		mesh->joints.resize(vertexCount);
		mesh->weights.resize(vertexCount);
	}
	mesh->m_Indices.resize(indexCount);
	mesh->m_TriangleCount = indexCount / 3;
	
//	for (int i = 0; i < mesh->m_SubMeshCount; ++i)
////	for (auto& info : mesh->m_SubMeshInfos)
//	{
//		auto& info = mesh->m_SubMeshInfos[i];
//		//printf("[ImportMesh]submesh: %d %d %d %d\n", i, info.StartIndex, info.Length, info.VertexOffset);
//	}

	for (int i = 0; i < mesh->m_SubMeshCount; ++i)
	{
		auto& primitive = gltf_mesh.primitives[i];
		auto& info = mesh->m_SubMeshInfos[i];
		ImportPrimitive(mesh, model, info, primitive, skinned);
	}

//	int count = sizeof(decltype(mesh->m_Vertices)::value_type);
	
	assert(mesh->m_Vertices.size() == mesh->m_VertexCount);

	mesh->__Upload();
}


void ImportAnimation(AnimationClip* animation, const tinygltf::Animation& anim, const tinygltf::Model& model, const std::vector<GameObject*>& gos)
{
	//auto& anim = model.animations[0];
	auto channelCount = anim.channels.size();
	animation->curves.resize(channelCount);
	for (int i = 0; i < channelCount; ++i)
	{
		auto& curve = animation->curves[i];
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

		auto& inputAccessor = model.accessors[sampler.input];
		float mintime = (float)inputAccessor.minValues[0];
		float maxtime = (float)inputAccessor.maxValues[0];
		

		LoadBuffer(model, sampler.input, curve.input);
		LoadBuffer(model, sampler.output, curve.output);

#if LOAD_GLTF_AS_LHS
		if (curve.type == AnimationCurveType::Translation)
		{
			for (int i = 2; i < curve.output.size(); i += 3)
				curve.output[i] *= -1;
		}
		else if (curve.type == AnimationCurveType::Rotation)
		{
			for (int i = 0; i < curve.output.size(); i += 4)
			{
				auto& o = curve.output;
				Quaternion q(o[i], o[i + 1], o[i + 2], o[i + 3]);
				RHS2LHS(q);
				o[i] = q.x;
				o[i + 1] = q.y;
				o[i + 2] = q.z;
				o[i + 3] = q.w;
			}
		}
#endif

		for (auto& i : curve.input)
			i -= mintime;
		animation->length = fmax(animation->length, maxtime-mintime);

		//if (curve.type == AnimationCurveType::Translation)
		//{
		//	for (int i = 0; i < curve.output.size(); i += 3)
		//	{
		//		curve.output[i] *= -1;
		//	}
		//}

		//assert(curve.input.size() == curve.output.size());
	}
}

void ImportSkin(Skin* skin, const tinygltf::Skin& gltf_skin, const tinygltf::Model& model, const std::vector<GameObject*>& gos)
{
	skin->name = gltf_skin.name;
	assert(gltf_skin.skeleton > 0);
	if (gltf_skin.skeleton > 0)
		skin->root = gos[gltf_skin.skeleton];
	auto boneCount = gltf_skin.joints.size();
	assert(boneCount < 256);
	skin->joints.reserve(boneCount);
	skin->inverseBindMatrices.resize(boneCount);
	for (int id : gltf_skin.joints)
	{
		skin->joints.push_back(gos[id]);
	}
	auto& accessor = model.accessors[gltf_skin.inverseBindMatrices];
	assert(accessor.type == TINYGLTF_TYPE_MAT4);
	assert(accessor.componentType == 5126);		// float
	assert(accessor.count == boneCount);
	auto& bufferView = model.bufferViews[accessor.bufferView];
	auto& buffer = model.buffers[bufferView.buffer];

	auto stride = accessor.ByteStride(bufferView);
	assert(stride == 16 * sizeof(float));
	auto offset = accessor.byteOffset + bufferView.byteOffset;
	auto ptr = buffer.data.data() + offset;
	memcpy(skin->inverseBindMatrices.data(), ptr, accessor.count * 16 * sizeof(float));

//	auto T = Matrix4x4::Scale(-1, 1, 1);
	for (auto& m : skin->inverseBindMatrices)
	{
		m = m.transpose();
		//m = T * m.inverse() * T;
		RHS2LHS(m);
	}

	//auto T = Matrix4x4::Scale(-1, 1, 1);
	// set bind pose
	//for (int i = 0; i < boneCount; ++i)
	//{
	//	//skin->joints[i]->GetTransform()->SetLocalMatrix(skin->inverseBindMatrices[i]);
	//	auto bone = skin->joints[i]->GetTransform();
	//	auto& bindpose = skin->inverseBindMatrices[i];
	//	//bindpose = T * bindpose.inverse() * T;
	//	bindpose = bone->GetWorldToLocalMatrix() * skin->root->GetTransform()->GetLocalToWorldMatrix();
	//	bindpose = T * bindpose * T;
	//}
}



Material* ImportMaterial(const tinygltf::Material& gltf_material,
						 const Model& model,
						 const tinygltf::Model& gltf_model)
{
	Material* mat = nullptr;

	
	if (In(gltf_material.extensions, "KHR_materials_pbrSpecularGlossiness"))
	{
		const auto& ext = Get(gltf_material.extensions, "KHR_materials_pbrSpecularGlossiness");
		assert(ext.Get("diffuseTexture").Get("index").IsInt());
		mat = Material::Clone(Material::TextureMaterial);
		int id = ext.Get("diffuseTexture").Get("index").Get<int>();
		int imageId = gltf_model.textures[id].source;
		auto img = model.images[imageId];
		mat->SetTexture("_MainTex", img);
	}
	else
	{
		mat = Material::Clone(Material::pbrMetallicRoughness);
		if (In(gltf_material.values, "baseColorTexture"))
		{
			int id = Get(gltf_material.values, "baseColorTexture").TextureIndex();
			int imageId = gltf_model.textures[id].source;
			auto img = model.images[imageId];
			mat->SetTexture("baseColorTexture", img);
		}
		if (In(gltf_material.values, "baseColorFactor"))
		{
			auto color = Get(gltf_material.values, "baseColorFactor").ColorFactor();
			Vector4 vcolor((float)color[0], (float)color[1], (float)color[2], (float)color[3]);
			mat->SetVector("baseColorFactor", vcolor);
		}
	}

	//else
	//{
	//	mat = Material::Clone(Material::ColorMaterial);
	//}
	return mat;
}


inline bool EndsWith(const std::string& s, const std::string& end)
{
	if (s.size() < end.size())
		return false;
	return s.substr(s.size() - end.size()) == end;
}

#include <FishEngine/Texture.hpp>


// TODO: temp
Model * current_model = nullptr;

bool gltfLoadImageData(tinygltf::Image *image, std::string *err, std::string *warn,
				   int req_width, int req_height, const unsigned char *bytes,
				   int size, void *)
{
	bgfx::TextureHandle texture = loadTexture2((void*)bytes, size, "unknown.ext");
	assert(bgfx::isValid(texture));
	current_model->images.push_back(texture);
	return true;
}

bool gltfLoadImageData_empty(tinygltf::Image *image, std::string *err, std::string *warn,
					   int req_width, int req_height, const unsigned char *bytes,
					   int size, void *)
{
	return true;
}

std::vector<Mesh*> FishEngine::ModelUtil::LoadMeshesFromGLTF(const std::string & filePath)
{
	Model model;
	GLTFLoadFlags flags;
	flags.loadCamera = false;
	flags.loadMateirals = false;
	flags.loadMeshes = true;
	flags.loadNodes = false;
	model.flags = flags;
	//tinygltf::Model& gltf_model = model.gltfModel;
	tinygltf::Model gltf_model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = false;
	if (EndsWith(filePath, ".gltf"))
		ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, filePath);
	else if (EndsWith(filePath, ".glb"))
		ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, filePath);
	else
		abort();

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
		abort();
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
		abort();
	}

	// load all meshes
	for (int i = 0; i < gltf_model.meshes.size(); ++i)
	{
		Mesh* mesh = new Mesh();
		ImportMesh(mesh, gltf_model, gltf_model.meshes[i]);
		model.meshes.push_back(mesh);
	}

	return model.meshes;
}

GameObject* ModelUtil::FromGLTF(const std::string& filePath, const GLTFLoadFlags& flags, Scene* scene)
{
	if (flags.loadNodes)
	{
		assert(scene != nullptr);
	}

	Model model;
	model.flags = flags;
	//tinygltf::Model& gltf_model = model.gltfModel;
	tinygltf::Model gltf_model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	
	current_model = &model;
	
	if (flags.loadMateirals)
	{
		loader.SetImageLoader(gltfLoadImageData, nullptr);
	}
	else
	{
		loader.SetImageLoader(gltfLoadImageData_empty, nullptr);
	}

	bool ret = false;
	if (EndsWith(filePath, ".gltf"))
		ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, filePath);
	else if (EndsWith(filePath, ".glb"))
		ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, filePath);
	else
		abort();

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
		abort();
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
		abort();
	}

	model.rootGameObject = scene->CreateGameObject();
	model.rootGameObject->name = "Root";
	
	model.materials.reserve(gltf_model.materials.size());
	if (flags.loadMateirals)
	{
		// loade all materials
		for (auto& m : gltf_model.materials)
		{
			auto mat = ImportMaterial(m, model, gltf_model);
			model.materials.push_back(mat);
		}
	}
	else
	{
		for (auto& m : gltf_model.materials)
		{
			model.materials.push_back(Material::ColorMaterial);
		}
	}

	// load all meshes
	for (int i = 0; i < gltf_model.meshes.size(); ++i)
	{
		Mesh* mesh = new Mesh();
		ImportMesh(mesh, gltf_model, gltf_model.meshes[i]);
		model.meshes.push_back(mesh);
	}

	// build node hierarchy
	auto nodeCount = gltf_model.nodes.size();
	model.nodes.reserve(nodeCount);
	//for (auto& node : gltf_model.nodes)
	for (int i = 0; i < nodeCount; ++i)
	{
		auto& node = gltf_model.nodes[i];
		auto go = scene->CreateGameObject();
		model.nodes.push_back(go);

		if (!node.name.empty())
			go->name = node.name;
		else
			go->name = "node" + std::to_string(i);

		auto mtx = node.matrix;
		if (mtx.size() > 0)
		{
			Matrix4x4 m;
			for (int i = 0; i < 16; ++i)
			{
				m.m[i % 4][i / 4] = (float)mtx[i];
			}
			auto t = go->GetTransform();
			RHS2LHS(m);
			t->SetLocalMatrix(m);
		}
		else
		{
			auto& p = node.translation;
			if (p.size() > 0)
			{
				//go->GetTransform()->SetLocalPosition(-p[0], p[1], p[2]);
				Vector3 pos((float)p[0], (float)p[1], (float)p[2]);
				RHS2LHS(pos);
				go->GetTransform()->SetLocalPosition(pos);
			}

			auto& s = node.scale;
			if (s.size() > 0)
			{
				go->GetTransform()->SetLocalScale((float)s[0], (float)s[1], (float)s[2]);
			}

			auto& r = node.rotation;
			if (r.size() > 0)
			{
				auto q = Quaternion((float)r[0], (float)r[1], (float)r[2], (float)r[3]);
				RHS2LHS(q);
				go->GetTransform()->SetLocalRotation(q);
			}
		}
	}

	// set transform parent
	for (int nodeID = 0; nodeID < gltf_model.nodes.size(); ++nodeID)
	{
		auto& node = gltf_model.nodes[nodeID];
		GameObject* go = model.nodes[nodeID];
		for (int childID : node.children)
		{
			model.nodes[childID]->GetTransform()->SetParent(go->GetTransform(), false);
		}
	}


	// load animations
	if (gltf_model.animations.size() > 0)
	{
		Animator* animator = scene->GameObjectAddComponent<Animator>(model.rootGameObject);
		for (auto& anim : gltf_model.animations)
		{
			auto clip = new AnimationClip();
			ImportAnimation(clip, anim, gltf_model, model.nodes);
			animator->m_Clips.push_back(clip);
		}
		animator->m_CurrentClipIndex = 0;
	}

	// load skins
	if (gltf_model.skins.size() > 0)
	{
		model.skins.reserve(gltf_model.skins.size());
		for (auto& gltf_skin : gltf_model.skins)
		{
			Skin* skin = new Skin();
			model.skins.push_back(skin);
			ImportSkin(skin, gltf_skin, gltf_model, model.nodes);
		}
	}


	for (int i = 0; i < gltf_model.nodes.size(); ++i)
	{
		auto& node = gltf_model.nodes[i];
		auto go = model.nodes[i];
		if (node.mesh >= 0)
		{
			Renderable* r = scene->GameObjectAddComponent<Renderable>(go);
			r->m_Mesh = model.meshes[node.mesh];
			auto& gltf_mesh = gltf_model.meshes[node.mesh];
//			r->material = model.materials[gltf_mesh.primitives[0].material];
			r->m_Materials.reserve(gltf_mesh.primitives.size());
			for (auto& p : gltf_mesh.primitives)
			{
				int id = p.material;
				if (id == -1)
					r->m_Materials.push_back(Material::ColorMaterial);
				else
					r->m_Materials.push_back(model.materials[id]);
			}

			if (node.skin >= 0)
			{
				r->m_Skin = model.skins[node.skin];
			}
		}
	}

	//	auto& img = model.images[0].image;
	////	auto tex = TextureUtils::LoadTextureFromMemory(img.data(), img.size());
	//	auto tex = loadTexture2(img.data(), img.size(), "from/gltf", BGFX_TEXTURE_NONE, nullptr, nullptr);

	assert(gltf_model.scenes.size() > 0);
	
	int defaultSceneId = gltf_model.defaultScene;
	if (defaultSceneId == -1)
		defaultSceneId = 0;
	auto& defaultScene = gltf_model.scenes[defaultSceneId];
	auto rootT = model.rootGameObject->GetTransform();
	for (int nodeID : defaultScene.nodes)
	{
		GameObject* go = model.nodes[nodeID];
		assert(go->GetTransform()->GetParent() == nullptr);
		go->GetTransform()->SetParent(rootT, false);
	}

//	PrintHierarchy(rootT, 0);

	return model.rootGameObject;
}
