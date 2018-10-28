#pragma once

#include "Object.hpp"
#include <vector>
//#include <bgfx/bgfx.h>
#include <map>
#include <string>
#include "Math.hpp"
#include "GraphicsAPI.hpp"

namespace FishEngine
{
	class Texture;
	
	struct MaterialProperties
	{
		std::map<std::string, Vector4> vec4s;
		std::map<std::string, Texture*> textures;
	};

	class Shader;
	class Texture;

	class Material : public Object
	{
	public:
		
		void SetShader(Shader* shader);
		Shader* GetShader() const { return m_Shader; }
		
		void SetVector(const std::string& name, const Vector4& value);
		void SetTexture(const std::string& name, Texture* value);
		void BindUniforms() const;
		
		static Material* Clone(Material* mat);
		
		static void StaticInit();
		inline static Material* ColorMaterial = nullptr;
		inline static Material* TextureMaterial = nullptr;
		inline static Material* ErrorMaterial = nullptr;
		inline static Material* pbrMetallicRoughness = nullptr;
	//	inline static Material* pbrMetallicRoughness_skinned = nullptr;
		
//	protected:
		Shader* m_Shader = nullptr;
//		std::map<std::string, std::pair<bgfx::UniformHandle, bgfx::UniformInfo>> m_UniformInfos;
		MaterialProperties m_MaterialProperties;
		
		ShaderHandle m_VertexShader;
		ShaderHandle m_PixelShader;
	};
	
}
