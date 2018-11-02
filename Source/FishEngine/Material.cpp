#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Texture.hpp>

#include <cassert>


using namespace FishEngine;

bool IsInernalUniform(const char* name)
{
	if (strcmp(name, "lightDir") == 0)
		return true;
	return false;
}


void Material::SetShader(Shader* shader)
{
	m_Shader = shader;
#if 0
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
#endif
	for (auto& arg : shader->m_VertexShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::Custom)
		{
			for (auto& u : arg.uniforms)
			{
				if (u.dataType == ShaderDataType::Float4)
				{
					m_MaterialProperties.vec4s[u.name] = Vector4(1, 1, 1, 1);
				}
				else if (u.dataType == ShaderDataType::Float)
				{
					m_MaterialProperties.vec4s[u.name] = Vector4(1, 1, 1, 1);
				}
			}
		}
	}
	
	for (auto& arg : shader->m_FragmentShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::Custom)
		{
			for (auto& u : arg.uniforms)
			{
				if (u.dataType == ShaderDataType::Float4)
				{
					m_MaterialProperties.vec4s[u.name] = Vector4(1, 1, 1, 1);
				}
				else if (u.dataType == ShaderDataType::Float)
				{
					m_MaterialProperties.vec4s[u.name] = Vector4(1, 1, 1, 1);
				}
			}
		}
	}
	for (auto& t : shader->m_FragmentShaderSignature.textures)
	{
		m_MaterialProperties.textures[t.name] = Texture::s_WhiteTexture;
	}
}


void Material::SetFloat(const char* name, float value)
{
	m_MaterialProperties.vec4s[name].x = value;
}


void Material::SetVector(const std::string& name, const Vector4& value)
{
#if 0
//	assert(m_UniformInfos.find(name) != m_UniformInfos.end());
	if (m_UniformInfos.find(name) != m_UniformInfos.end())
		m_MaterialProperties.vec4s[name] = value;
	else
		printf("Material::SetVector: %s not found!\n", name.c_str());
#endif
}

void Material::SetTexture(const std::string& name, Texture* value)
{
#if 0
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
#endif
	m_MaterialProperties.textures[name] = value;
}


void Material::BindUniforms() const
{
#if 0
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
#endif
}

Material* Material::Clone(Material* mat)
{
	assert(mat != nullptr);
	Material* m = new Material();
	m->name = mat->name;
	m->m_Shader = mat->m_Shader;
//	m->m_VertexShader = mat->m_VertexShader;
//	m->m_PixelShader = mat->m_PixelShader;
//	m->m_UniformInfos = mat->m_UniformInfos;
	m->m_MaterialProperties = mat->m_MaterialProperties;
	return m;
}

#include <FishEngine/Assets.hpp>

Material* CreateMaterialFromShadersDir(const char* shader_name)
{
//	auto vs = "Shaders/runtime/" + std::string(shader_name) + "_vs.bin";
//	auto fs = "Shaders/runtime/" + std::string(shader_name) + "_fs.bin";
//	vs = FISHENGINE_ROOT + vs;
//	fs = FISHENGINE_ROOT + fs;
//	auto shader = ShaderUtil::Compile(vs, fs);
	auto shader = ShaderUtil::CompileFromShaderName(shader_name);
	auto mat = new Material;
	mat->SetShader(shader);
	mat->name = shader_name;
	return mat;
}

#include <FishEngine/Texture.hpp>

void Material::StaticInit()
{
//	ColorMaterial = CreateMaterialFromShadersDir("color");
//	TextureMaterial = CreateMaterialFromShadersDir("Texture");
//	ErrorMaterial = CreateMaterialFromShadersDir("Error");
//
//	pbrMetallicRoughness = CreateMaterialFromShadersDir("pbrMetallicRoughness");
//	pbrMetallicRoughness->SetVector("baseColorFactor", Vector4::one);
//	pbrMetallicRoughness->SetTexture("baseColorTexture", Texture::s_WhiteTexture);
//	pbrMetallicRoughness_skinned = CreateMaterialFromShadersDir("pbrMetallicRoughness_skinned");
	
	ColorMaterial = new Material();
	ColorMaterial->name = "Color";
	ColorMaterial->SetShader(ShaderUtil::CompileFromShaderName("Color"));
	
	pbrMetallicRoughness = new Material();
	pbrMetallicRoughness->name = "pbrMetallicRoughness";
	pbrMetallicRoughness->SetShader(ShaderUtil::CompileFromShaderName("pbrMetallicRoughness"));
	pbrMetallicRoughness->SetFloat("Metallic", 0);
	pbrMetallicRoughness->SetFloat("Roughness", 0.5f);
	pbrMetallicRoughness->SetFloat("Specular", 0.5f);
}
