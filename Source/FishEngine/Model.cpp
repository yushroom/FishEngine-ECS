#include <FishEngine/Model.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Components/Animation.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Components/Transform.hpp>

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

//using tinygltf::Model;

void GetVector3(
	Vector3& v,
	int idx,
	const tinygltf::Buffer& buffer,
	const tinygltf::BufferView& bufferView,
	const tinygltf::Accessor& accessor
)
{
	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	size_t stride = bufferView.byteStride == 0 ? sizeof(Vector3) : bufferView.byteStride;
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

	size_t offset = accessor.byteOffset + bufferView.byteOffset;
	auto ptr = buffer.data.data() + offset;
	memcpy(out_buffer.data(), ptr, accessor.count * s * sizeof(float));
}


void RHS2LHS(Vector3& v)
{
	v.z *= -1;
}

void RHS2LHS(Quaternion& q)
{
	auto euler = q.eulerAngles();
	euler.x *= -1;
	q = Quaternion::Euler(euler);
}

void RHS2LHS(Matrix4x4& m)
{
	Vector3 t, s;
	Quaternion r;
	Matrix4x4::Decompose(m, &t, &r, &s);
	m.SetTRS(t, r, s);
}

void ImportMesh(Mesh* mesh, const tinygltf::Model& model, tinygltf::Mesh& gltf_mesh)
{
	assert(gltf_mesh.primitives.size() == 1);
	auto& primitive = gltf_mesh.primitives[0];

	bool withTangent = false;
	bool withJoints = false;
	bool withWeights = false;
	for (auto& pair : primitive.attributes)
	{
		if (pair.first == "TANGENT")
			withTangent = true;
		else if (pair.first == "JOINTS_0")
			withJoints = true;
		else if (pair.first == "WEIGHTS_0")
			withWeights = true;
	}

	int id = primitive.attributes["POSITION"];
	auto& position_accessor = model.accessors[id];
	auto& position_bufferView = model.bufferViews[position_accessor.bufferView];
	auto& position_buffer = model.buffers[position_bufferView.buffer];


	id = primitive.attributes["NORMAL"];
	auto& normal_accessor = model.accessors[id];
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
		//RHS2LHS(v.position);
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

	if (withJoints)
	{
		mesh->joints.resize(mesh->m_vertexCount);
		id = primitive.attributes["JOINTS_0"];
		auto& accessor = model.accessors[id];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];

		assert(accessor.count == mesh->m_vertexCount);

		int offset = accessor.byteOffset + bufferView.byteOffset;
		auto ptr = buffer.data.data() + offset;

		if (accessor.componentType == 5123)	// unsigned short
		{
			auto p = (unsigned short*)ptr;
			for (int i = 0; i < accessor.count; ++i)
			{
				mesh->joints[i].x = *p; ++p;
				mesh->joints[i].y = *p; ++p;
				mesh->joints[i].z = *p; ++p;
				mesh->joints[i].w = *p; ++p;
			}
		}
		else if (accessor.componentType == 5121)	// unsigned byte
		{
			auto p = (unsigned char*)ptr;
			for (int i = 0; i < accessor.count; ++i)
			{
				mesh->joints[i].x = *p; ++p;
				mesh->joints[i].y = *p; ++p;
				mesh->joints[i].z = *p; ++p;
				mesh->joints[i].w = *p; ++p;
			}
		}
		else
		{
			abort();
		}
	}

	if (withWeights)
	{
		mesh->weights.resize(mesh->m_vertexCount);
		id = primitive.attributes["WEIGHTS_0"];
		auto& accessor = model.accessors[id];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];

		assert(accessor.componentType == 5126);		// float
		assert(accessor.count == mesh->m_vertexCount);

		int offset = accessor.byteOffset + bufferView.byteOffset;
		auto ptr = buffer.data.data() + offset;
		memcpy(mesh->weights.data(), ptr, accessor.count * 4 * sizeof(float));
	}


	//assert(mesh->indices.size() * sizeof())
	//memcpy(mesh->indices.data(), ptr, indices_bufferView.byteLength);

	mesh->m_VertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(mesh->vertices.data(),
		sizeof(PUNTVertex)*mesh->vertices.size()),
		PUNTVertex::ms_decl
	);

	int size = 1;	// in bytes
	switch (indices_accessor.componentType)
	{
	case 5120:	// byte
	case 5121:	// unsigned byte
		size = 1; break;
	case 5122:	// short
	case 5123:	// unsigned short
		size = 2; break;
	case 5125:	// unsigned int
	case 5126:	// float
		size = 4; break;
	default:
		abort();
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

	mesh->m_IndexBuffer = bgfx::createIndexBuffer(bgfx::copy(ptr, byteLen));
}

void ImportAnimator(Animation* animation, const tinygltf::Animation& anim, const tinygltf::Model& model, const std::vector<ECS::GameObject*>& gos)
{
	//auto& anim = model.animations[0];
	int channelCount = anim.channels.size();
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
		float maxtime = inputAccessor.maxValues[0];
		animation->length = fmax(animation->length, maxtime);

		LoadBuffer(model, sampler.input, curve.input);
		LoadBuffer(model, sampler.output, curve.output);

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

void ImportSkin(Skin* skin, const tinygltf::Skin& gltf_skin, const tinygltf::Model& model, const std::vector<ECS::GameObject*>& gos)
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

	int offset = accessor.byteOffset + bufferView.byteOffset;
	auto ptr = buffer.data.data() + offset;
	memcpy(skin->inverseBindMatrices.data(), ptr, accessor.count * 16 * sizeof(float));

	for (auto& m : skin->inverseBindMatrices)
		m = m.transpose();

	//auto T = Matrix4x4::Scale(-1, 1, 1);
	//// set bind pose
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

void PrintHierarchy(Transform* t, int indent)
{
	for (int i = 0; i < indent; ++i)
		putchar(' ');
	printf(t->m_GameObject->m_Name.c_str());
	printf("    local pos: %s  pos: %s", t->GetLocalPosition().ToString().c_str(), t->GetPosition().ToString().c_str());
	putchar('\n');
	for (auto c : t->GetChildren())
	{
		PrintHierarchy(c, indent + 2);
	}
}

Model ModelUtil::FromGLTF(const char* filePath, ECS::Scene* scene)
{
	Model model;
	//tinygltf::Model& gltf_model = model.gltfModel;
	tinygltf::Model gltf_model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	//bool ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, filePath);
	bool ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, filePath); // for binary glTF(.glb)

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
	model.rootGameObject->m_Name = "Root";

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
			go->m_Name = node.name;
		else
			go->m_Name = "node" + std::to_string(i);

		auto mtx = node.matrix;
		if (mtx.size() > 0)
		{
			Matrix4x4 m;
			for (int i = 0; i < 16; ++i)
			{
				m.m[i % 4][i / 4] = mtx[i];
			}
			auto t = go->GetTransform();
			//t->SetLocalMatrix(m);
			//auto euler = t->GetLocalEulerAngles();
		}
		else
		{
			auto& p = node.translation;
			if (p.size() > 0)
			{
				//go->GetTransform()->SetLocalPosition(-p[0], p[1], p[2]);
				go->GetTransform()->SetLocalPosition(p[0], p[1], p[2]);
			}

			auto& s = node.scale;
			if (s.size() > 0)
			{
				go->GetTransform()->SetLocalScale(s[0], s[1], s[2]);
			}

			auto& r = node.rotation;
			if (r.size() > 0)
			{
				auto q = Quaternion(r[0], r[1], r[2], r[3]);
				//auto euler = q.eulerAngles();
				//euler.x = -euler.x;
				//auto q = Quaternion::Euler(euler);
				go->GetTransform()->SetLocalRotation(q);
			}
		}
	}

	// set transform parent
	for (int nodeID = 0; nodeID < gltf_model.nodes.size(); ++nodeID)
	{
		auto& node = gltf_model.nodes[nodeID];
		ECS::GameObject* go = model.nodes[nodeID];
		for (int childID : node.children)
		{
			model.nodes[childID]->GetTransform()->SetParent(go->GetTransform(), false);
		}
	}

	// load animations
	if (gltf_model.animations.size() > 0)
	{
		auto& anim = gltf_model.animations[0];
		Animation* animation = scene->GameObjectAddComponent<Animation>(model.rootGameObject);
		ImportAnimator(animation, anim, gltf_model, model.nodes);
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
			r->mesh = model.meshes[node.mesh];

			if (node.skin >= 0)
			{
				r->skin = model.skins[node.skin];
			}
		}
	}

	//	auto& img = model.images[0].image;
	////	auto tex = TextureUtils::LoadTextureFromMemory(img.data(), img.size());
	//	auto tex = loadTexture2(img.data(), img.size(), "from/gltf", BGFX_TEXTURE_NONE, nullptr, nullptr);

	auto& defaultScene = gltf_model.scenes[gltf_model.defaultScene];
	auto rootT = model.rootGameObject->GetTransform();
	for (int nodeID : defaultScene.nodes)
	{
		ECS::GameObject* go = model.nodes[nodeID];
		assert(go->GetTransform()->GetParent() == nullptr);
		go->GetTransform()->SetParent(rootT, false);
	}

	PrintHierarchy(rootT, 0);

	return model;
}
