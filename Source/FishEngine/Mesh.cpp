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

#if 0

void PUNTVertex::StaticInit()
{
	ms_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Float)
		.end();
	assert(PUNTVertex::ms_decl.getStride() == sizeof(PUNTVertex));

	s_P_decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
	
	s_PC_decl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
	.end();
}

bgfx::VertexDecl PUNTVertex::ms_decl;
bgfx::VertexDecl PUNTVertex::s_P_decl;

void Mesh::StaticInit()
{
	PUNTVertex::StaticInit();

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
	m_VertexBuffer = bgfx::createVertexBuffer(bgfxHelper::MakeRef(m_Vertices), PUNTVertex::ms_decl);
	m_IndexBuffer = bgfx::createIndexBuffer(bgfxHelper::MakeRef(m_Indices), BGFX_BUFFER_INDEX32);
}

void Mesh::Bind(int subMeshIndex/* = -1*/, int bgfxStream/* = 0*/)
{
	if (subMeshIndex < -1 || subMeshIndex >= m_SubMeshCount)
	{
		abort();
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
#endif

void Mesh::StaticInit()
{
	auto meshes = ModelUtil::LoadMeshesFromGLTF(FISHENGINE_ROOT "Assets/Models/Unity2Skfb.gltf");
	assert(meshes.size() == 6);

	Quad = meshes[0];
	Cube = meshes[1];
	Capsule = meshes[2];
	Cylinder = meshes[3];
	Sphere = meshes[4];
	Plane = meshes[5];
}

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <FishEngine/Render/d3dx12.h>
#include <FishEngine/Render/Helpers.h>

void UpdateBufferResource(
	ComPtr<ID3D12Device2> device,
	ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize, const void* bufferData,
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
{
	size_t bufferSize = numElements * elementSize;

	// Create a committed resource for the GPU resource in a default heap.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(pDestinationResource)));

	// Create a committed resource for the upload.
	if (bufferData)
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(),
			*pDestinationResource, *pIntermediateResource,
			0, 0, 1, &subresourceData);
	}
}

#include <FishEngine/Render/Application.h>
#include <FishEngine/Render/CommandQueue.h>
#include <FishEngine/Render/MeshImpl.hpp>
#include <DirectXMath.h>
using DirectX::XMFLOAT3;

//// Vertex data for a colored cube.
//struct VertexPosColor
//{
//	XMFLOAT3 Position;
//	XMFLOAT3 Color;
//};
//
//static VertexPosColor g_Vertices[8] = {
//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
//	{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
//	{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
//	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
//	{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
//	{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
//	{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
//	{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
//};
//
//static WORD g_Indicies[36] =
//{
//	0, 1, 2, 0, 2, 3,
//	4, 6, 5, 4, 7, 6,
//	4, 5, 1, 4, 1, 0,
//	3, 2, 6, 3, 6, 7,
//	1, 5, 6, 1, 6, 2,
//	4, 0, 3, 4, 3, 7
//};

void Mesh::__Upload()
{
	assert(m_Impl == nullptr);
	m_Impl = new MeshImpl();

	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	// Upload vertex buffer data.
	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	UpdateBufferResource(device, commandList,
		&m_Impl->m_VertexBuffer, &intermediateVertexBuffer,
		m_Vertices.size(), sizeof(PUNTVertex), m_Vertices.data());

	// Create the vertex buffer view.
	m_Impl->m_VertexBufferView.BufferLocation = m_Impl->m_VertexBuffer->GetGPUVirtualAddress();
	m_Impl->m_VertexBufferView.SizeInBytes = bgfxHelper::Sizeof(m_Vertices);
	m_Impl->m_VertexBufferView.StrideInBytes = sizeof(PUNTVertex);

	// Upload index buffer data.
	ComPtr<ID3D12Resource> intermediateIndexBuffer;
	UpdateBufferResource(device, commandList,
		&m_Impl->m_IndexBuffer, &intermediateIndexBuffer,
		m_Indices.size(), sizeof(uint32_t), m_Indices.data());

	// Create index buffer view.
	m_Impl->m_IndexBufferView.BufferLocation = m_Impl->m_IndexBuffer->GetGPUVirtualAddress();
	m_Impl->m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_Impl->m_IndexBufferView.SizeInBytes = bgfxHelper::Sizeof(m_Indices);

	// TODO
	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
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
