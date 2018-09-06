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
//				m_UniformInfos.resize(count);
			std::vector<bgfx::UniformHandle> uniforms(count);
			bgfx::getShaderUniforms(shader->m_FragmentShader, uniforms.data(), count);
			
			for (int i = 0; i < count; ++i)
			{
				auto& u = uniforms[i];
				//					auto& info = m_UniformInfos[i];
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
	assert(m_UniformInfos.find(name) != m_UniformInfos.end());
	m_MaterialProperties.vec4s[name] = value;
}

void Material::SetTexture(const std::string& name, bgfx::TextureHandle value)
{
//	assert(m_UniformInfos.find(name) != m_UniformInfos.end());
	if (m_UniformInfos.find(name) != m_UniformInfos.end())
		m_MaterialProperties.textures[name] = value;
	else
		printf("Material::SetTexture: %s not found!\n", name.c_str());
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
				bgfx::setTexture(texCount, handle, value);
			}
			texCount ++;
		}
		
	}
}

#include <FishEngine/Assets.hpp>

void Material::StaticInit()
{
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/color_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/color_fs.bin";
		auto shader = ShaderUtil::Compile(vs, fs);
		Default = new Material;
		Default->SetShader(shader);
	}
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/Texture_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/Texture_fs.bin";
		auto shader = ShaderUtil::Compile(vs, fs);
		Texture = new Material;
		Texture->SetShader(shader);
	}
}
