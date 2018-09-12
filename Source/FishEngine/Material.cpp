#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>

#include <cassert>


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

void Material::SetTexture(const std::string& name, bgfx::TextureHandle value)
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
				assert(bgfx::isValid(value));
				bgfx::setTexture(texCount, handle, value);
			}
			texCount ++;
		}
		
	}
}

Material* Material::Clone(Material* mat)
{
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

	auto white = TextureUtils::LoadTexture(FISHENGINE_ROOT "Assets/Textures/white.png");

	pbrMetallicRoughness = CreateMaterialFromShadersDir("pbrMetallicRoughness");
	pbrMetallicRoughness->SetVector("baseColorFactor", Vector4::one);
	pbrMetallicRoughness->SetTexture("baseColorTexture", white);
}
