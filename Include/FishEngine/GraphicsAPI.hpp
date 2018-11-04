#pragma once

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include "Math/Vector2.hpp"
#include "Math/Matrix4x4.hpp"

#define FISHENGINE_METAL 1
#if FISHENGINE_METAL
//#   include <Metal/Metal.h>
#else
#include <CommandContext.h>
#endif

struct GLFWwindow;

namespace FishEngine
{
	class Mesh;
	class Material;
	class Camera;
	class Light;
	class Texture;
	
    struct Handle
    {
    //protected:
    public:
        uint16_t idx = 0;
        
        virtual bool IsValid() const { return idx != 0; }
    };

    struct BufferHandle : public Handle {};
    struct VertexBufferHandle : public BufferHandle {};
    struct IndexBufferHandle : public BufferHandle {};
	struct DynamicVertexBufferHandle : public BufferHandle {};
//  struct TextureBufferHandle : public BufferHandle {};
    struct FrameBufferHandle : public BufferHandle {};
    struct ShaderHandle : public Handle {};
    struct TextureHandle : public Handle {};

	enum class VertexAttrib
	{
		Position = 0,
		TexCoord0 = 1,
		Normal = 2,
		Tangernt = 3,
		Color = 4,
	};

	enum class VertexAttribType
	{
		Float,
	};

    struct VertexDecl
    {
    public:
        
		VertexDecl& Begin();
		VertexDecl& Add(VertexAttrib attrib, int count, VertexAttribType type);
		void End();
		
		int GetStride() const
		{
			assert(m_Valid && m_Index != 0);
			return this->m_Stride; 
		}
		
		int GetIndex() const
		{
			return m_Index;
		}
		
		bool IsValid() const
		{
			return m_Valid;
		}
		
	private:
		bool m_Valid = false;
		int m_AttribCount = 0;
        int m_Stride = 0;
		int m_Index = 0;
    };

    struct Memory
    {
        const void* data = nullptr;
        int byteSize = 0;   // bytes
		
		template<class T>
		static Memory FromVectorArray(const std::vector<T>& varray)
		{
			Memory m;
			m.data = (void *)varray.data();
			m.byteSize = varray.size() * sizeof(T);
			return m;
		}
    };

    enum class MeshIndexType
    {
//      UInt8,
        UInt16,
        UInt32
    };

    VertexBufferHandle CreateVertexBuffer(const Memory& data, const VertexDecl& decl);

    IndexBufferHandle CreateIndexBuffer(const Memory& data, MeshIndexType type);
	
	DynamicVertexBufferHandle CreateDynamicVertexBufferHandle(int vertexCount, const VertexDecl& decl);
	
    ShaderHandle CreateShader(const char* functionName);
	
	TextureHandle CreateTexture(const Memory& data, int width, int height);
	
	void UpdateDynamicVertexBuffer(DynamicVertexBufferHandle handle, int startVertex, const Memory& data);
    
    enum class TextureFormat
    {
        R8G8B8,
        R8G8B8A8,
		BGRA8Unorm,
		Depth32Float,
    };
    
    FrameBufferHandle CreateFrameBuffer(TextureFormat foramt);
	
	// MTLDataType
	enum class ShaderDataType
	{
		Float,
		Float4,
		Float4x4,
	};
	
	struct ShaderUniform
	{
		std::string name;
		int offset;
		ShaderDataType dataType;
	};
	
	enum class ShaderUniformBufferType
	{
		// user defined
		Custom = 0,
		
		// vertex buffer is also a shader argument in metal
		VertexBuffer,
		
		// internal
		PerDrawUniforms,
		PerCameraUniforms,
		LightingUniforms
	};
	
	struct ShaderUniformBuffer
	{
		std::string name;
		int index;
		bool isInternal = false;
		int size;
		ShaderUniformBufferType type;
		std::vector<ShaderUniform> uniforms;
	};
	
	struct ShaderUniformTexture
	{
		std::string name;
		int bindIndex;
	};
	
	struct ShaderUniformSampler
	{
		std::string name;
		int bindIndex;
	};
	
	struct ShaderUniformInfo
	{
		std::string name;
		ShaderDataType dataType;
	};
	
	struct ShaderUniformSignature
	{
		std::vector<ShaderUniformBuffer> arguments;
		std::vector<ShaderUniformTexture> textures;
		std::vector<ShaderUniformSampler> samplers;
	};
	
	class Shader;
	
	void internal_ReflectShader(Shader* shader);
	
    
//    class RenderPipelineStateImpl;
	
    class RenderPipelineState
    {
    public:
		RenderPipelineState();
		
		void SetShader(Shader* s) { m_Shader = s; }
		void SetVertexDecl(VertexDecl vertexDecl)
		{
			assert(vertexDecl.IsValid());
			m_VertexDecl = vertexDecl;
		}
		void SetColorAttachment0Format(TextureFormat format) { m_ColorAttachment0Format = format; }
		void SetDepthAttachmentFormat(TextureFormat format) { m_DepthAttachmentFormat = format; }
		
		void Create();
		
//    protected:

//        std::unique_ptr<RenderPipelineStateImpl> impl;
		
		int m_Index = 0;
		
	private:
		bool m_Created = false;
		Shader* m_Shader = nullptr;
		VertexDecl m_VertexDecl;
		
		TextureFormat m_ColorAttachment0Format = TextureFormat::BGRA8Unorm;
		TextureFormat m_DepthAttachmentFormat = TextureFormat::Depth32Float;
		
//		ShaderUniformSignature vertexShaderSignature;
//		ShaderUniformSignature fragmentShaderSignature;
		
//      TextureHandle renderTarget;
#if FISHENGINE_METAL
#else
        GraphicsPSO GetGraphicsPS() const;
#endif
    };
	
	
	class RenderPass
	{
	public:
		RenderPipelineState rps;
	};
	
    
    struct Viewport
    {
        float originX;
        float originY;
        float width;
        float height;
        float znear;
        float zfar;
    };
    
    class CommandQueueImpl;
    class CommandList;
    
    class CommandQueue
    {
    public:
//      CommandQueue() :
//          context(GraphicsContext::Begin(L"Temp"))
//      {
//
//      }
        CommandQueue();
        
        CommandList* GetCommandList();
        
    private:
        std::string m_Label;
        //std::unique_ptr<CommandQueueImpl> impl;
#ifdef FISHENGINE_METAL
//        id<MTLCommandQueue> commandQueue;
#else
        GraphicsContext& context;
#endif
    };
	
	
	enum class CullMode
	{
		None = 0,		// Do not cull.
		Front = 1,		// Cull front-facing primitives.
		Back = 2,		// Cull back-facing primitives.
	};
    
    class CommandListImpl;
    
    class CommandList
    {
    public:
        void SetViewport(const Viewport& viewport);
        void SetRenderPipelineState(RenderPipelineState* psd);
        void SetVertexBuffer(VertexBufferHandle vbh);
        
        void DrawPrimitives();
        
//  private:
//      std::unique_ptr<CommandListImpl> impl;
#ifdef FISHENGINE_METAL
//        id<MTLCommandEncoder> commandEncoder;
#endif
		
		std::string label;
		RenderPipelineState* psd = nullptr;
		CullMode cullMode = CullMode::Back;
        Viewport viewport;
        VertexBufferHandle vb;
    };
	
	void InitGraphicsAPI(GLFWwindow* window);
	void ResetGraphicsAPI();
	
	void ImguiNewFrame();
	void ImguiRender();
	void ImGuiDrawTexture(Texture* texture, const Vector2& size);
	
	void SetModelMatrix(const Matrix4x4& model);
	void SetViewProjectionMatrix(const Matrix4x4& view, const Matrix4x4& proj);
	void SetCamera(Camera* camera);
	void SetLight(Light* light);
	
	void BeginFrame();
	void EndFrame();
	void ClearColorDepthBuffer();
	
	void BeginPass(const RenderPipelineState& rps, bool clear = false);
	void EndPass();
	
	void SetVertexBuffer(VertexBufferHandle handle);
	void SetVertexBuffer(DynamicVertexBufferHandle handle);
	void Draw(Mesh* mesh, Material* material, int submeshID);
	void Submit(Material* material);
}
