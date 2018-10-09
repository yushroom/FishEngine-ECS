#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Texture.hpp>
#include <cassert>

using namespace FishEngine;

#if 0

bool IsInernalUniform(const char* name)
{
	if (strcmp(name, "lightDir") == 0)
		return true;
	return false;
}


void Material::SetShader(Shader* shader)
{
	m_Shader = shader;
	if (shader != nullptr)
	{
		auto count = bgfx::getShaderUniforms(shader->m_FragmentShader);
		
		if (count != 0)
		{
			std::vector<bgfx::UniformHandle> uniforms(count);
			bgfx::getShaderUniforms(shader->m_FragmentShader, uniforms.data(), count);
			
			for (int i = 0; i < count; ++i)
			{
				auto& u = uniforms[i];

				bgfx::UniformInfo info;
				bgfx::getUniformInfo(u, info);
				//printf("%s\n", info.name);
				m_UniformInfos[info.name] = std::make_pair(u, info);
				if (IsInernalUniform(info.name))
					continue;
				if (info.type == bgfx::UniformType::Vec4)
					m_MaterialProperties.vec4s[info.name] = Vector4(1, 1, 1, 1);
				else
					m_MaterialProperties.textures[info.name] = Texture::s_WhiteTexture;
			}
		}
	}
}


void Material::SetVector(const std::string& name, const Vector4& value)
{
//	assert(m_UniformInfos.find(name) != m_UniformInfos.end());
	if (m_UniformInfos.find(name) != m_UniformInfos.end())
		m_MaterialProperties.vec4s[name] = value;
	else
		printf("Material::SetVector: %s not found!\n", name.c_str());
}

void Material::SetTexture(const std::string& name, Texture* value)
{
//	assert(m_UniformInfos.find(name) != m_UniformInfos.end());
	//m_MaterialProperties.textures[name] = value;
	if (m_UniformInfos.find(name) != m_UniformInfos.end())
		m_MaterialProperties.textures[name] = value;
	else
	{
		printf("Material::SetTexture: %s not found!\n", name.c_str());
		auto handle = bgfx::createUniform(name.c_str(), bgfx::UniformType::Int1);
		bgfx::UniformInfo info;
		strcpy(info.name, name.c_str());
		info.type = bgfx::UniformType::Int1;
		info.num = 1;
		m_UniformInfos[name] = std::make_pair(handle, info);
		m_MaterialProperties.textures[name] = value;
	}
	
}

void Material::BindUniforms() const
{
	int texCount = 0;
	for (auto& pair : m_UniformInfos)
	{
		const std::string& name = pair.first;
		auto& p = pair.second;
		auto& handle = p.first;
		auto& info = p.second;
		if (info.type == bgfx::UniformType::Vec4)
		{
			auto it = m_MaterialProperties.vec4s.find(name);
			if (it != m_MaterialProperties.vec4s.end())
			{
				const Vector4& value = it->second;
				bgfx::setUniform(handle, value.data());
			}
		}
		else if (info.type == bgfx::UniformType::Int1)
		{
			auto it = m_MaterialProperties.textures.find(name);
			if (it != m_MaterialProperties.textures.end())
			{
				auto& value = it->second;
				assert(bgfx::isValid(value->GetHandlde()));
				bgfx::setTexture(texCount, handle, value->GetHandlde());
			}
			texCount ++;
		}
		
	}
}

Material* Material::Clone(Material* mat)
{
	assert(mat != nullptr);
	Material* m = new Material();
	m->name = mat->name;
	m->m_Shader = mat->m_Shader;
	m->m_UniformInfos = mat->m_UniformInfos;
	m->m_MaterialProperties = mat->m_MaterialProperties;
	return m;
}

#include <FishEngine/Assets.hpp>

Material* CreateMaterialFromShadersDir(const char* shader_name)
{
	auto vs = "Shaders/runtime/" + std::string(shader_name) + "_vs.bin";
	auto fs = "Shaders/runtime/" + std::string(shader_name) + "_fs.bin";
	vs = FISHENGINE_ROOT + vs;
	fs = FISHENGINE_ROOT + fs;
	auto shader = ShaderUtil::Compile(vs, fs);
	auto mat = new Material;
	mat->SetShader(shader);
	mat->name = shader_name;
	return mat;
}

#include <FishEngine/Texture.hpp>

void Material::StaticInit()
{
	ColorMaterial = CreateMaterialFromShadersDir("color");
	TextureMaterial = CreateMaterialFromShadersDir("Texture");
	ErrorMaterial = CreateMaterialFromShadersDir("Error");

	pbrMetallicRoughness = CreateMaterialFromShadersDir("pbrMetallicRoughness");
//	pbrMetallicRoughness->SetVector("baseColorFactor", Vector4::one);
//	pbrMetallicRoughness->SetTexture("baseColorTexture", Texture::s_WhiteTexture);
	
//	pbrMetallicRoughness_skinned = CreateMaterialFromShadersDir("pbrMetallicRoughness_skinned");
}

#endif

#include <FishEngine/Render/Helpers.h>
#include <d3dcompiler.h>

#include <d3d12.h>
#include <wrl.h>
#include <FishEngine/Render/Application.h>
#include <FishEngine/Render/d3dx12.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using DirectX::XMMATRIX;
#include <FishEngine/Render/ShaderImpl.hpp>

void Material::StaticInit()
{
	auto shader = new Shader();
	shader->m_Impl = new ShaderImpl();
	ThrowIfFailed(D3DReadFileToBlob(L"I:\\FishEngine-ECS\\Shaders\\hlsl\\runtime\\Simple_vs.cso", &shader->m_Impl->m_VertexShaderBlob));
	ThrowIfFailed(D3DReadFileToBlob(L"I:\\FishEngine-ECS\\Shaders\\hlsl\\runtime\\Simple_ps.cso", &shader->m_Impl->m_PixelShaderBlob));

	ColorMaterial = new Material();
	ColorMaterial->SetShader(shader);

	auto device = Application::Get().GetDevice();

	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// Serialize the root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&shader->m_Impl->m_RootSignature)));
}