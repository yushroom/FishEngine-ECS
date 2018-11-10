#include <FishEngine/GraphicsAPI.hpp>
//#include "../../GraphicsPlatform.hpp"

#include <vector>
#include <unordered_map>

#include <MetalKit/MetalKit.h>

#include <FishEngine/Mesh.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/ECS/GameObject.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Texture.hpp>

#if USE_GLFW
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA 1
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>
#else
#include <imgui.h>
#include <imgui_impl_osx.h>
#include <imgui_impl_metal.h>
#endif


using namespace FishEngine;

constexpr int MaxBuffersInFlight = 3;

id<MTLDevice> g_device;
#if USE_GLFW
GLFWwindow* g_window = nullptr;
CAMetalLayer* g_metalLayer = nil;
#else
MTKView* g_MTKView;
#endif
id<MTLTexture> g_mainDepthBuffer;
id<MTLTexture> g_mainStencilBuffer;
id<CAMetalDrawable> g_currentFrameDrawable;
MTLRenderPassDescriptor* g_imguiRenderPassDesc = nil;
id<MTLCommandQueue> g_commandQueue;
id<MTLCommandBuffer> g_currentCommandBuffer;
id<MTLLibrary> g_defaultLibrary;
//id<MTLRenderPipelineState> g_pipelineState;
//NSArray<id<MTLBuffer>> *g_uniformBuffers;
id<MTLDepthStencilState> g_normalDepthStencilState;

int g_frameNumber = 0;
int g_currentBufferIndex = 0;
dispatch_semaphore_t g_inFlightSemaphore;

//MTLRenderPassDescriptor* g_mainRenderPassDesc;
id<MTLRenderCommandEncoder> g_mainPassCommandEncoder;

id<MTLSamplerState> g_sampler;

VertexDecl g_fullVertexDecl;

RenderPipelineState g_rps;

template<class T, int MaxCount>
struct TArray
{
	int Next()
	{
		assert(currentIndex < MaxCount-1);
		++currentIndex;
		return currentIndex;
	}
	
	T operator[](int index) const
	{
		assert(index <= currentIndex);
		return arrays[index];
	}
	
	T& operator[](int index)
	{
		assert(index <= currentIndex);
		return arrays[index];
	}
	
private:
	int currentIndex = 0;
	T arrays[MaxCount];
};

static TArray<id<MTLBuffer>, 256> g_buffers;
static TArray<id<MTLFunction>, 128> g_shaders;
static TArray<id<MTLTexture>, 128> g_textures;
static TArray<MTLVertexDescriptor*, 16> g_vertexDescriptors;
static TArray<id<MTLRenderPipelineState>, 64> g_renderPipelineStates;

std::unordered_map<std::string, Shader*> g_CompiledShaders;


struct PerDrawUniforms
{
	Matrix4x4 MATRIX_MVP;
	Matrix4x4 MATRIX_MV;
	Matrix4x4 MATRIX_M;
	Matrix4x4 MATRIX_IT_M;
};

struct PerCameraUniforms
{
	Matrix4x4 MATRIX_P;
	Matrix4x4 MATRIX_V;
	Matrix4x4 MATRIX_I_V;
	Matrix4x4 MATRIX_VP;
	
	Vector4 WorldSpaceCameraPos;     // w = 1, not used
	Vector4 WorldSpaceCameraDir;     // w = 0, not used
};

struct LightingUniforms
{
	Vector4 LightPos;        // w=1 not used
	Vector4 LightDir;        // w=0 not used
};


static PerDrawUniforms g_perDrawUniforms;
static PerCameraUniforms g_perCameraUniforms;
static LightingUniforms g_lightinUniforms;


//namespace FishEngine
//{
//	class RenderPipelineStateImpl
//	{
//	public:
//		id<MTLRenderPipelineState> m_rps;
//	};
//}


void FishEngine::InitGraphicsAPI()
{
	g_inFlightSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);
	
#if USE_GLFW
	assert(g_window != nullptr);
	NSWindow* nsWindow = (NSWindow*)glfwGetCocoaWindow(g_window);
	NSView* view = nsWindow.contentView;
	[view setWantsLayer:YES];
	g_metalLayer = [CAMetalLayer layer];
	[view setLayer:g_metalLayer];
	g_device = MTLCreateSystemDefaultDevice();
	g_metalLayer.device = g_device;
	g_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	g_metalLayer.magnificationFilter = kCAFilterNearest;
	auto rect = view.bounds;
	g_metalLayer.frame = rect;
#endif
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("/Users/yushroom/program/FishEngine-ECS/Assets/Fonts/DroidSans.ttf", 16.0f);
	ImGui_ImplMetal_Init(g_device);
#if USE_GLFW
	ImGui_ImplGlfw_InitForOpenGL(g_window, false);
#else
//	ImGui_ImplOSX_Init();
#endif
	
	
//	NSString* osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
//	if ([osxMode  isEqual: @"Dark"])
//		ImGui::StyleColorsDark();
//	else
//		ImGui::StyleColorsLight();
	ImGui::StyleColorsLight();
	
	g_commandQueue = [g_device newCommandQueue];
	[g_commandQueue setLabel:@"MainCmdQueue"];
	
#if 1
	g_defaultLibrary = [g_device newDefaultLibrary];
#else
	NSError *libraryError = NULL;
	NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"shader" ofType:@"metallib"];
	g_defaultLibrary = [g_device newLibraryWithFile:libraryFile error:&libraryError];
	if (libraryError != NULL)
	{
		NSLog(@"Error: failed to load shader library: %@", libraryError);
		abort();
	}
#endif
	for (NSString* name in g_defaultLibrary.functionNames)
	{
		NSLog(@"%@\n", name);
	}
	
	if (![g_device isDepth24Stencil8PixelFormatSupported])
	{
		puts("Depth24Stencil8PixelFormat is not supported.");
	}
	
	
	g_fullVertexDecl.Begin()
		.Add(VertexAttrib::Position, 3, VertexAttribType::Float)
		.Add(VertexAttrib::TexCoord0, 2, VertexAttribType::Float)
		.Add(VertexAttrib::Normal, 3, VertexAttribType::Float)
		.Add(VertexAttrib::Tangernt, 4, VertexAttribType::Float)
		.Add(VertexAttrib::Color, 3, VertexAttribType::Float)
	.End();
	
	PUNTVertex::StaticInit();
	
	// before shader compile
	
//	id<MTLFunction> vertexFunction = [g_defaultLibrary newFunctionWithName:@"Normal_VS"];
//	id<MTLFunction> fragmentFunction = [g_defaultLibrary newFunctionWithName:@"Normal_PS"];
	Shader* normalShader = Shader::Find("pbrMetallicRoughness");
	g_rps.SetShader(normalShader);
	g_rps.SetVertexDecl(PUNTVertex::ms_decl);
	g_rps.Create("pbrMetallicRoughness");
	
//	g_pipelineState = g_renderPipelineStates[rps.m_Index];
	
//	id<MTLBuffer> uniformBuffersCArray[AAPLMaxBuffersInFlight];
	
//	for (int i = 0; i < AAPLMaxBuffersInFlight; ++i)
//	{
//		uniformBuffersCArray[i] = [g_device newBufferWithLength:sizeof(Uniforms) options:MTLResourceStorageModeShared];
//		uniformBuffersCArray[i].label = [NSString stringWithFormat:@"UniformBuffer%d", i];
//	}
//	
//	g_uniformBuffers = [[NSArray alloc] initWithObjects:uniformBuffersCArray count:AAPLMaxBuffersInFlight];
	
	MTLDepthStencilDescriptor* depthStateDesc = [MTLDepthStencilDescriptor new];
	depthStateDesc.label = @"normal";
	depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
	depthStateDesc.depthWriteEnabled = YES;
	g_normalDepthStencilState = [g_device newDepthStencilStateWithDescriptor:depthStateDesc];
	
	MTLTextureDescriptor* depthBufferDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8 width:800*2 height:600*2 mipmapped:NO];
	depthBufferDesc.resourceOptions = MTLResourceStorageModePrivate;
	depthBufferDesc.usage = MTLTextureUsageRenderTarget;
	g_mainDepthBuffer = [g_device newTextureWithDescriptor:depthBufferDesc];
	g_mainDepthBuffer.label = @"Main Depth buffer";
	
	MTLSamplerDescriptor* samplerDesc = [MTLSamplerDescriptor new];
	samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
	samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
	samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
	samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
	samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
	g_sampler = [g_device newSamplerStateWithDescriptor:samplerDesc];
}

void FishEngine::ResetGraphicsAPI(int framebufferWidth, int framebufferHeight)
{
	printf("ResetGraphicsAPI %d, %d\n", framebufferWidth, framebufferHeight);
//	int fbw, fbh;
	int fbw = framebufferWidth;
	int fbh = framebufferHeight;
#if USE_GLFW
//	glfwGetFramebufferSize(g_window, &fbw, &fbh);
	auto w = (NSWindow*)glfwGetCocoaWindow(g_window);
	auto view = w.contentView;
	auto layer = (CAMetalLayer*)view.layer;
	auto bounds = view.bounds;
	layer.frame = bounds;
	layer.drawableSize = CGSizeMake(fbw, fbh);
//	viewportSize.x = fbw;
//	viewportSize.y = fbh;
#endif
	
	MTLTextureDescriptor* depthBufferDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8 width:fbw height:fbh mipmapped:NO];
	depthBufferDesc.resourceOptions = MTLResourceStorageModePrivate;
	depthBufferDesc.usage = MTLTextureUsageRenderTarget;
	g_mainDepthBuffer = [g_device newTextureWithDescriptor:depthBufferDesc];
	g_mainDepthBuffer.label = @"Main Depth buffer";
	
	MTLTextureDescriptor* stencilBufferDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatStencil8 width:fbw height:fbh mipmapped:NO];
	stencilBufferDesc.textureType = MTLTextureType2D;
	stencilBufferDesc.resourceOptions = MTLResourceStorageModePrivate;
	stencilBufferDesc.usage = MTLTextureUsageRenderTarget;
	g_mainStencilBuffer = [g_device newTextureWithDescriptor:stencilBufferDesc];
	g_mainStencilBuffer.label = @"Main Stencil Buffer";
}

//Matrix4x4 g_modelMat;
//Matrix4x4 g_viewMat;
//Matrix4x4 g_projMat;


void FishEngine::SetModelMatrix(const Matrix4x4& model)
{
//	g_modelMat = model;
	g_perDrawUniforms.MATRIX_M = model;
	g_perDrawUniforms.MATRIX_MV = g_perCameraUniforms.MATRIX_V * model;
	g_perDrawUniforms.MATRIX_IT_M = model.inverse().transpose();
	g_perDrawUniforms.MATRIX_MVP = g_perCameraUniforms.MATRIX_VP * g_perDrawUniforms.MATRIX_M;
}

void FishEngine::SetViewProjectionMatrix(const Matrix4x4& view, const Matrix4x4& proj)
{
//	g_viewMat = view;
//	g_projMat = proj;
	g_perCameraUniforms.MATRIX_V = view;
	g_perCameraUniforms.MATRIX_P = proj;
	g_perCameraUniforms.MATRIX_VP = proj * view;
	g_perCameraUniforms.MATRIX_I_V = view.inverse();
	g_perDrawUniforms.MATRIX_MV = view * g_perDrawUniforms.MATRIX_M;
	g_perDrawUniforms.MATRIX_MVP = g_perCameraUniforms.MATRIX_VP * g_perDrawUniforms.MATRIX_M;
}

void FishEngine::SetCamera(Camera* camera)
{
	auto t = camera->GetGameObject()->GetTransform();
	g_perCameraUniforms.WorldSpaceCameraPos.Set( t->GetPosition().normalized(), 1 );
	g_perCameraUniforms.WorldSpaceCameraDir.Set( t->GetForward(), 0);
}

void FishEngine::SetLight(Light* light)
{
	assert(light);
	auto t = light->GetGameObject()->GetTransform();
	g_lightinUniforms.LightPos.Set( t->GetPosition().normalized(), 1 );
	g_lightinUniforms.LightDir.Set( t->GetForward(), 0);
}

void FishEngine::BeginFrame()
{
	// Wait to ensure only AAPLMaxBuffersInFlight are getting processed by any stage in the Metal pipeline (App, Metal, Drivers, GPU, etc)
	dispatch_semaphore_wait(g_inFlightSemaphore, DISPATCH_TIME_FOREVER);
	
	// Create a new command buffer for each render pass to the current drawable
	g_currentCommandBuffer = [g_commandQueue commandBuffer];
	g_currentCommandBuffer.label = @"Frame";
	
	// Add completion hander which signal _inFlightSemaphore when Metal and the GPU has fully
	//   finished processing the commands we're encoding this frame.  This indicates when the
	//   dynamic buffers, that we're writing to this frame, will no longer be needed by Metal
	//   and the GPU.
	__block dispatch_semaphore_t block_sema = g_inFlightSemaphore;
	[g_currentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_semaphore_signal(block_sema);
	}];
	
#if USE_GLFW
	g_currentFrameDrawable = [g_metalLayer nextDrawable];
#else
	g_currentFrameDrawable = [g_MTKView currentDrawable];
#endif
	
	g_frameNumber ++;
	g_currentBufferIndex = (g_currentBufferIndex+1) % MaxBuffersInFlight;
	
	BeginPass(g_rps, true);
}

void FishEngine::ClearColorDepthBuffer()
{
	MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
	passDesc.colorAttachments[0].texture = g_currentFrameDrawable.texture;
	passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
	passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
	passDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.63, 0.81, 0, 1);
	passDesc.depthAttachment.texture = g_mainDepthBuffer;
	passDesc.depthAttachment.loadAction = MTLLoadActionClear;
	passDesc.depthAttachment.clearDepth = 0.0;
	id<MTLCommandBuffer> cmdBuffer = [g_commandQueue commandBuffer];
	id<MTLRenderCommandEncoder> encoder = [cmdBuffer renderCommandEncoderWithDescriptor:passDesc];
	encoder.label = @"CleadPass";
	[encoder endEncoding];
}

void FishEngine::BeginPass(const RenderPipelineState& rps, bool clear)
{
	EndPass();
	assert(rps.m_Index != 0);
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = g_currentFrameDrawable.texture;
	if (clear)
	{
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.81, 0.81, 0.81, 1);
	}
	else
	{
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
	}
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = g_mainDepthBuffer;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.stencilAttachment.texture = g_mainDepthBuffer;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	
	g_mainPassCommandEncoder = [g_currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
	[g_mainPassCommandEncoder setLabel:[NSString stringWithUTF8String:rps.GetName().c_str()]];
	//	[renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
	[g_mainPassCommandEncoder setCullMode:MTLCullModeBack];
	//	MTLViewport vp = {0, 0, 1600, 1200, 0, 1};
	//	[g_mainPassCommandEncoder setViewport:vp];
	[g_mainPassCommandEncoder setDepthStencilState:g_normalDepthStencilState];
	[g_mainPassCommandEncoder setRenderPipelineState:g_renderPipelineStates[rps.m_Index]];
}

void FishEngine::EndPass()
{
	[g_mainPassCommandEncoder endEncoding];
	g_mainPassCommandEncoder = nil;
}

void FishEngine::SetVertexBuffer(VertexBufferHandle handle)
{
	[g_mainPassCommandEncoder setVertexBuffer:g_buffers[handle.idx] offset:0 atIndex:0];
}

void FishEngine::SetVertexBuffer(DynamicVertexBufferHandle handle)
{
	[g_mainPassCommandEncoder setVertexBuffer:g_buffers[handle.idx] offset:0 atIndex:0];
}

void FishEngine::Submit(Material* mat, int vertexCount)
{
	for (auto& arg : mat->m_Shader->m_VertexShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::PerDrawUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_perDrawUniforms length:sizeof(PerDrawUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::PerCameraUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_perCameraUniforms length:sizeof(g_perCameraUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::LightingUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_lightinUniforms length:sizeof(g_lightinUniforms) atIndex:arg.index];
		}
	}
	for (auto& arg : mat->m_Shader->m_FragmentShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::PerDrawUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_perDrawUniforms length:sizeof(PerDrawUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::PerCameraUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_perCameraUniforms length:sizeof(g_perCameraUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::LightingUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_lightinUniforms length:sizeof(g_lightinUniforms) atIndex:arg.index];
		}
		else
		{
			assert(arg.size % 4 == 0);
			std::vector<float> buffer(arg.size/4);
			auto ptr = (std::byte*)buffer.data();
			for (auto& u : arg.uniforms)
			{
				int size = 1;
				switch (u.dataType)
				{
					case ShaderDataType::Float:
						size = 4; break;
					case ShaderDataType::Float4:
						size = 4*4; break;
					default:
						abort();
				}
				auto& v = mat->m_MaterialProperties.vec4s[u.name];
				memcpy(ptr+u.offset, v.data(), size);
			}
			[g_mainPassCommandEncoder setFragmentBytes:ptr length:arg.size atIndex:arg.index];
		}
	}
	
	for (auto& t : mat->m_Shader->m_FragmentShaderSignature.textures)
	{
		Texture* tex = mat->m_MaterialProperties.textures[t.name];
		[g_mainPassCommandEncoder setFragmentTexture:g_textures[tex->m_Handle.idx] atIndex:t.bindIndex];
	}
	
	for (auto& s : mat->m_Shader->m_FragmentShaderSignature.samplers)
	{
		[g_mainPassCommandEncoder setFragmentSamplerState:g_sampler atIndex:s.bindIndex];
	}
	

	//	[g_mainPassCommandEncoder setFragmentBytes:&u_color length:16 atIndex:2];
//	[g_mainPassCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//										 indexCount:mesh->m_TriangleCount*3
//										  indexType:MTLIndexTypeUInt32
//										indexBuffer:g_buffers[ib.idx]
//								  indexBufferOffset:0];
	[g_mainPassCommandEncoder drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:vertexCount];
}

void FishEngine::Draw(FishEngine::Mesh* mesh, FishEngine::Material* mat, int submeshID)
{
	auto vb = mesh->m_VertexBuffer;
	auto ib = mesh->m_IndexBuffer;
	
	SetVertexBuffer(vb);
	
	for (auto& arg : mat->m_Shader->m_VertexShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::PerDrawUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_perDrawUniforms length:sizeof(PerDrawUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::PerCameraUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_perCameraUniforms length:sizeof(g_perCameraUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::LightingUniforms)
		{
			[g_mainPassCommandEncoder setVertexBytes:&g_lightinUniforms length:sizeof(g_lightinUniforms) atIndex:arg.index];
		}
	}
	for (auto& arg : mat->m_Shader->m_FragmentShaderSignature.arguments)
	{
		if (arg.type == ShaderUniformBufferType::PerDrawUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_perDrawUniforms length:sizeof(PerDrawUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::PerCameraUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_perCameraUniforms length:sizeof(g_perCameraUniforms) atIndex:arg.index];
		}
		else if (arg.type == ShaderUniformBufferType::LightingUniforms)
		{
			[g_mainPassCommandEncoder setFragmentBytes:&g_lightinUniforms length:sizeof(g_lightinUniforms) atIndex:arg.index];
		}
		else
		{
			assert(arg.size % 4 == 0);
			std::vector<float> buffer(arg.size/4);
			auto ptr = (std::byte*)buffer.data();
			for (auto& u : arg.uniforms)
			{
				int size = 1;
				switch (u.dataType)
				{
					case ShaderDataType::Float:
						size = 4; break;
					case ShaderDataType::Float4:
						size = 4*4; break;
					default:
						abort();
				}
				auto& v = mat->m_MaterialProperties.vec4s[u.name];
				memcpy(ptr+u.offset, v.data(), size);
			}
			[g_mainPassCommandEncoder setFragmentBytes:ptr length:arg.size atIndex:arg.index];
		}
	}
	
	for (auto& t : mat->m_Shader->m_FragmentShaderSignature.textures)
	{
		Texture* tex = mat->m_MaterialProperties.textures[t.name];
		[g_mainPassCommandEncoder setFragmentTexture:g_textures[tex->m_Handle.idx] atIndex:t.bindIndex];
	}
	
	for (auto& s : mat->m_Shader->m_FragmentShaderSignature.samplers)
	{
		[g_mainPassCommandEncoder setFragmentSamplerState:g_sampler atIndex:s.bindIndex];
	}
	
//	[g_mainPassCommandEncoder setVertexBytes:&g_perDrawUniforms length:sizeof(PerDrawUniforms) atIndex:1];
//	[g_mainPassCommandEncoder setFragmentBytes:&u_color length:16 atIndex:2];
	if (submeshID == -1)
	{
		[g_mainPassCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
								  indexCount:mesh->m_TriangleCount*3
								   indexType:MTLIndexTypeUInt32
								 indexBuffer:g_buffers[ib.idx]
						   indexBufferOffset:0];
	}
	else
	{
		auto& info = mesh->m_SubMeshInfos[submeshID];
		[g_mainPassCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
											 indexCount:info.Length
											  indexType:MTLIndexTypeUInt32
											indexBuffer:g_buffers[ib.idx]
									  indexBufferOffset:info.StartIndex*4];
	}
}

void FishEngine::EndFrame()
{
//	[g_mainPassCommandEncoder endEncoding];
	[g_currentCommandBuffer presentDrawable:g_currentFrameDrawable];
	[g_currentCommandBuffer commit];
}

void FishEngine::ImguiNewFrame()
{
//	auto frameDrawable = [g_metalLayer nextDrawable];
	MTLRenderPassDescriptor * renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = g_currentFrameDrawable.texture;
//	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
//	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 1.0, 1.0, 1.0);
//	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	ImGui_ImplMetal_NewFrame(renderPassDescriptor);
#if USE_GLFW
	ImGui_ImplGlfw_NewFrame();
#else
	ImGui_ImplOSX_NewFrame(g_MTKView);
#endif
	ImGui::NewFrame();
	
	g_imguiRenderPassDesc = renderPassDescriptor;
}

void FishEngine::ImguiRender()
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	id<MTLRenderCommandEncoder> renderEncoder = [g_currentCommandBuffer renderCommandEncoderWithDescriptor:g_imguiRenderPassDesc];
	renderEncoder.label = @"ImGui";
	
	ImGui_ImplMetal_RenderDrawData(drawData, g_currentCommandBuffer, renderEncoder);
	
	[renderEncoder endEncoding];
}


VertexBufferHandle FishEngine::CreateVertexBuffer(const Memory& data, const VertexDecl& decl)
{
	int size = decl.GetStride();
	assert(data.byteSize % size == 0);
//	id<MTLBuffer> buffer = [g_device newBufferWithBytes:data.data
//												 length:data.byteSize
//												options:MTLResourceStorageModePrivate];
	id<MTLBuffer> buffer = [g_device newBufferWithLength:data.byteSize options:MTLResourceStorageModeShared];
	memcpy(buffer.contents, data.data, data.byteSize);
	VertexBufferHandle handle;
	handle.idx = g_buffers.Next();
	g_buffers[handle.idx] = buffer;
	return handle;
}

DynamicVertexBufferHandle FishEngine::CreateDynamicVertexBufferHandle(int vertexCount, const VertexDecl& decl)
{
	int byteSize = vertexCount * decl.GetStride();
	id<MTLBuffer> buffer = [g_device newBufferWithLength:byteSize options:MTLResourceStorageModeShared];
	DynamicVertexBufferHandle handle;
	handle.idx = g_buffers.Next();
	g_buffers[handle.idx] = buffer;
	return handle;
}


IndexBufferHandle FishEngine::CreateIndexBuffer(const Memory& data, MeshIndexType type)
{
//	int size = decl.GetVertexSize();
//	assert(data.byteSize % size == 0);
//	id<MTLBuffer> buffer = [g_device newBufferWithBytes:data.data
//												 length:data.byteSize
//												options:MTLResourceStorageModePrivate];
	id<MTLBuffer> buffer = [g_device newBufferWithLength:data.byteSize options:MTLResourceStorageModeShared];
	memcpy(buffer.contents, data.data, data.byteSize);
	IndexBufferHandle handle;
	handle.idx = g_buffers.Next();
	g_buffers[handle.idx] = buffer;
	return handle;
}

TextureHandle FishEngine::CreateTexture(const Memory& data, int width, int height)
{
#if 0
	id<MTLTexture> texture;
	id<MTLTexture> texture2; // intermediate
	
	{
		MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:NO];
		textureDesc.resourceOptions = MTLResourceStorageModeManaged;
		texture2 = [g_device newTextureWithDescriptor:textureDesc];
		
		NSUInteger bytesPerRow = 4 * static_cast<NSUInteger>(width);
		
		MTLRegion region = {
			{0, 0, 0},				// MTLOrigin
			{static_cast<NSUInteger>(width), static_cast<NSUInteger>(height), 1}		// MTLSize
		};
		
		[texture2 replaceRegion:region mipmapLevel:0 withBytes:data.data bytesPerRow:bytesPerRow];
	}
	
	{
		MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:YES];
		textureDesc.resourceOptions = MTLResourceStorageModePrivate;
		texture = [g_device newTextureWithDescriptor:textureDesc];
	}
	
	MTLSize size = {static_cast<NSUInteger>(width), static_cast<NSUInteger>(height), 1};
	
	
	id<MTLCommandBuffer> commandBuffer = [g_commandQueue commandBuffer];
	id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
	
	[commandEncoder copyFromTexture:texture2 sourceSlice:0 sourceLevel:0 sourceOrigin:{0, 0, 0} sourceSize:size toTexture:texture destinationSlice:0 destinationLevel:0 destinationOrigin:{0, 0, 0}];
	[commandEncoder generateMipmapsForTexture:texture];
	[commandEncoder endEncoding];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];
#endif
	// see https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/ResourceOptions.html
	MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:YES];
	textureDesc.resourceOptions = MTLResourceStorageModeManaged;
	textureDesc.usage = MTLTextureUsageShaderRead;
	id<MTLTexture> texture = [g_device newTextureWithDescriptor:textureDesc];
	
	NSUInteger bytesPerRow = 4 * static_cast<NSUInteger>(width);
	
	MTLRegion region = {
		{0, 0, 0},				// MTLOrigin
		{static_cast<NSUInteger>(width), static_cast<NSUInteger>(height), 1}		// MTLSize
	};
	
	[texture replaceRegion:region mipmapLevel:0 withBytes:data.data bytesPerRow:bytesPerRow];
	
	id<MTLCommandBuffer> commandBuffer = [g_commandQueue commandBuffer];
	id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
	[commandEncoder generateMipmapsForTexture:texture];
	[commandEncoder endEncoding];
	[commandBuffer commit];
	
	
	TextureHandle handle;
	handle.idx = g_textures.Next();
	g_textures[handle.idx] = texture;
	return handle;
}

void FishEngine::UpdateDynamicVertexBuffer(DynamicVertexBufferHandle handle, int startVertex, const Memory& data)
{
	assert(handle.IsValid());
//	id<MTLBuffer> buffer = g_buffers[handle.idx];
//	assert(buffer.length == data.byteSize);
	if (g_buffers[handle.idx].length == data.byteSize)
	{
		memcpy(g_buffers[handle.idx].contents, data.data, data.byteSize);
	}
	else
	{
		g_buffers[handle.idx] = [g_device newBufferWithBytes:data.data length:data.byteSize options:MTLResourceStorageModeShared];
	}
//	memcpy(buffer.contents, data.data, data.byteSize);
}

//id<MTLBuffer> FishEngine::GetVertexBuffer(VertexBufferHandle handle)
//{
//	assert(handle.IsValid());
//	return g_buffers[handle.idx];
//}
//
//CommandQueue::CommandQueue()
//{
////	impl = std::make_unique<CommandQueueImpl>();
//	commandQueue = [g_device newCommandQueue];
//}
//
//CommandQueueImpl::CommandQueueImpl()
//{
//	m_CommandQueue = [g_device newCommandQueue];
//}

VertexDecl& FishEngine::VertexDecl::Begin()
{
	assert(m_Index == 0);
	m_Index = g_vertexDescriptors.Next();
	MTLVertexDescriptor* desc = [[MTLVertexDescriptor alloc] init];
	g_vertexDescriptors[m_Index] = desc;
	return *this;
}

VertexDecl& FishEngine::VertexDecl::Add(VertexAttrib attrib, int count, VertexAttribType type)
{
	assert(m_Index != 0);
	auto a = g_vertexDescriptors[m_Index].attributes[static_cast<int>(attrib)];
	int size = 1;
	if (type == VertexAttribType::Float)
	{
		size = sizeof(float);
		switch (count)
		{
		case 1:
			a.format = MTLVertexFormatFloat; break;
		case 2:
			a.format = MTLVertexFormatFloat2; break;
		case 3:
			a.format = MTLVertexFormatFloat3; break;
		case 4:
			a.format = MTLVertexFormatFloat4; break;
		default:
			abort();
		}
	}
	else
	{
		abort();
	}

	a.offset = m_Stride;
//	a.bufferIndex = static_cast<int>(attrib);
	a.bufferIndex = 0;

	m_Stride += count*size;
	m_AttribCount++;
	return *this;
}

void FishEngine::VertexDecl::End()
{
	assert(m_Index != 0);

	auto desc = g_vertexDescriptors[m_Index];
	desc.layouts[0].stride = m_Stride;
	//desc.layouts[0].stepRate = 1;
	m_Valid = true;
}

bool DoesStringStartWith(const std::string& str, const std::string& prefix)
{
	int size2 = prefix.size();
	if (str.size() < size2)
		return false;
//	return str.substr(0, size2) == prefix;
	return strncmp(str.c_str(), prefix.c_str(), size2) == 0;
}

ShaderUniformBufferType IsInternalShaderUniformBuffer(const std::string& name)
{
	if (name == "PerDrawUniforms")
	{
		return ShaderUniformBufferType::PerDrawUniforms;
	}
	else if (name == "PerCameraUniforms")
	{
		return ShaderUniformBufferType::PerCameraUniforms;
	}
	else if (name == "LightingUniforms")
	{
		return ShaderUniformBufferType::LightingUniforms;
	}
	else if (name == "vertexBuffer.0")
	{
		return ShaderUniformBufferType::VertexBuffer;
	}
	return ShaderUniformBufferType::Custom;
};


ShaderUniformSignature GetShaderUniformSignature(NSArray <MTLArgument *> *args)
{
	ShaderUniformSignature signature;
//	puts("Reflect shader uniforms:");
	for (MTLArgument* arg in args)
	{
		printf("\targ: %s type: %lu index: %lu\n", arg.name.UTF8String, arg.type, (unsigned long)arg.index);
		
		if (arg.active)
		{
			if (arg.type == MTLArgumentTypeBuffer)
			{
				ShaderUniformBuffer ub;
				ub.name = arg.name.UTF8String;
				ub.index = arg.index;
				if (MTLDataTypeStruct == arg.bufferDataType)
				{
					printf("\t\tsize: %lu\n", (unsigned long)arg.bufferDataSize);
					ub.size = arg.bufferDataSize;
				}
				
				ub.type = IsInternalShaderUniformBuffer(ub.name);
				if (ub.type == ShaderUniformBufferType::Custom)
				{
					for (MTLStructMember* uniform in arg.bufferStructType.members)
					{
						const char* name = uniform.name.UTF8String;
						printf("\t\tuniform name: %s offset: %lu type: %lu\n", name, (unsigned long)uniform.offset, (unsigned long)uniform.dataType);
						ShaderUniform u;
						u.name = name;
						u.offset = uniform.offset;
						if (uniform.dataType == MTLDataTypeFloat)
							u.dataType = ShaderDataType::Float;
						else if (uniform.dataType == MTLDataTypeFloat4)
							u.dataType = ShaderDataType::Float4;
						else if (uniform.dataType == MTLDataTypeFloat4x4)
							u.dataType = ShaderDataType::Float4x4;
						ub.uniforms.push_back(u);
					}
				}
				
				signature.arguments.push_back(ub);
			}
			else if (arg.type == MTLArgumentTypeTexture)
			{
				ShaderUniformTexture t;
				t.name = arg.name.UTF8String;
				t.bindIndex = arg.index;
				signature.textures.push_back(t);
			}
			else if (arg.type == MTLArgumentTypeSampler)
			{
				ShaderUniformSampler s;
				s.name = arg.name.UTF8String;
				s.bindIndex = arg.index;
				signature.samplers.push_back(s);
			}
		}
	}
	
	for (int i = 0; i < signature.arguments.size(); ++i)
	{
		for (int j = i+1; j < signature.arguments.size(); ++j)
		{
			// different buffers share the same buffer index in shader
			assert( signature.arguments[i].index != signature.arguments[j].index );
		}
	}
	
	return signature;
}


void FishEngine::internal_ReflectShader(Shader* shader)
{
	MTLRenderPipelineDescriptor* psd = [[MTLRenderPipelineDescriptor alloc] init];
	psd.label = @"ReflectShaderPipeline";
	psd.vertexFunction = g_shaders[shader->m_VertexShader.idx];
	psd.fragmentFunction = g_shaders[shader->m_FragmentShader.idx];
#if USE_GLFW
	psd.colorAttachments[0].pixelFormat = g_metalLayer.pixelFormat;
#else
	psd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
#endif
//	psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	psd.vertexDescriptor = g_vertexDescriptors[g_fullVertexDecl.GetIndex()];	// real vertex decl is not known before reflection, so use full decl here
	
	NSError* error = NULL;
	
	MTLPipelineOption option = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
	MTLRenderPipelineReflection* reflection;
	id<MTLRenderPipelineState> rps = [g_device newRenderPipelineStateWithDescriptor:psd options:option reflection:&reflection error:&error];
	
	printf("  vertex arguments:\n");
	shader->m_VertexShaderSignature = GetShaderUniformSignature(reflection.vertexArguments);
	printf("  fragment arguments:\n");
	shader->m_FragmentShaderSignature = GetShaderUniformSignature(reflection.fragmentArguments);
	
	if (!rps)
	{
		NSLog(@"Failed to created pipeline state, error %@", error);
		abort();
		exit(1);
	}
}


namespace FishEngine
{
	ShaderHandle CreateShader(const char* functionName)
	{
		assert(g_defaultLibrary != nil);
		NSString* s = [NSString stringWithUTF8String:functionName];
		id<MTLFunction> f = [g_defaultLibrary newFunctionWithName:s];
		
		assert(f != nil);
		
		ShaderHandle h;
		h.idx = g_shaders.Next();
		g_shaders[h.idx] = f;
		return h;
	}
	
	
	RenderPipelineState::RenderPipelineState()
	{
	}
	
//	void RenderPipelineState::SetShader(Shader* s)
//	{
//		SetVertexShader( s->m_VertexShader );
//		SetFragmentShader( s->m_FragmentShader );
//	}
	
	void RenderPipelineState::Create(const char* name)
	{
//		assert(impl == nullptr);
		assert(!m_Created);
		assert(m_VertexDecl.IsValid());
		m_Name = name;
//		impl = std::make_unique<RenderPipelineStateImpl>();
		MTLRenderPipelineDescriptor* psd = [[MTLRenderPipelineDescriptor alloc] init];
		psd.label = [NSString stringWithUTF8String:name];
		psd.vertexFunction = g_shaders[m_Shader->m_VertexShader.idx];
		psd.fragmentFunction = g_shaders[m_Shader->m_FragmentShader.idx];
#if USE_GLFW
		psd.colorAttachments[0].pixelFormat = g_metalLayer.pixelFormat;
#else
		psd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
#endif
		psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		psd.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		
		psd.vertexDescriptor = g_vertexDescriptors[m_VertexDecl.GetIndex()];
		
		NSError* error = NULL;
		id<MTLRenderPipelineState> mtl_rps = [g_device newRenderPipelineStateWithDescriptor:psd error:&error];
#if 0
		MTLPipelineOption option = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
		MTLRenderPipelineReflection* reflection;
		impl->m_rps = [g_device newRenderPipelineStateWithDescriptor:psd options:option reflection:&reflection error:&error];
		
		printf("vertex arguments:\n");
		vertexShaderSignature = GetShaderUniformSignature(reflection.vertexArguments);
		printf("fragment arguments:\n");
		fragmentShaderSignature = GetShaderUniformSignature(reflection.fragmentArguments);
#endif
		if (!mtl_rps)
		{
			NSLog(@"Failed to created pipeline state, error %@", error);
			abort();
			exit(1);
		}
		
		m_Index = g_renderPipelineStates.Next();
		g_renderPipelineStates[m_Index] = mtl_rps;
		m_Created = true;
	}
}


void FishEngine::ImGuiDrawTexture(Texture* texture, const Vector2& size)
{
	int idx = texture->m_Handle.idx;
	auto t = g_textures[idx];
	ImGui::Image((__bridge ImTextureID)t, ImVec2(size.x, size.y));
}
