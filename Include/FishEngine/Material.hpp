#pragma once
#include "Shader.hpp"
#include <vector>
#include <bgfx/bgfx.h>
#include <map>
#include <string>
#include "Math.hpp"

struct MaterialProperties
{
	std::map<std::string, Vector4> vec4s;
	std::map<std::string, bgfx::TextureHandle> textures;
};

class Material : public Object
{
public:
	
	void SetShader(Shader* shader)
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
	
	Shader* GetShader() const { return m_Shader; }
	
	void SetVector(const std::string& name, const Vector4& value)
	{
		m_MaterialProperties.vec4s[name] = value;
	}
	void SetTexture(const std::string& name, bgfx::TextureHandle value)
	{
		m_MaterialProperties.textures[name] = value;
	}
	
	void BindUniforms() const
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
	
protected:
	Shader* m_Shader = nullptr;
	std::map<std::string, std::pair<bgfx::UniformHandle, bgfx::UniformInfo>> m_UniformInfos;
	MaterialProperties m_MaterialProperties;
};
