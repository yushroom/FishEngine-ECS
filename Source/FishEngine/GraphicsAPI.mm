#include <FishEngine/GraphicsAPI.hpp>
//#include "../../GraphicsPlatform.hpp"

#include <vector>
#include <unordered_map>

#include <MetalKit/MetalKit.h>

#include <FishEngine/Mesh.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Shader.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Transform.hpp>

using namespace FishEngine;

constexpr int MaxBuffersInFlight = 3;

GLFWwindow* g_window = nullptr;
id<MTLDevice> g_device;
CAMetalLayer* g_metalLayer = nil;
id<MTLTexture> g_mainDepthBuffer;
id<CAMetalDrawable> g_currentFrameDrawable;
MTLRenderPassDescriptor* g_imguiRenderPassDesc = nil;
id<MTLCommandQueue> g_commandQueue;
id<MTLCommandBuffer> g_currentCommandBuffer;
id<MTLLibrary> g_defaultLibrary;
id<MTLRenderPipelineState> g_pipelineState;
//NSArray<id<MTLBuffer>> *g_uniformBuffers;
id<MTLDepthStencilState> g_normalDepthStencilState;

int g_frameNumber = 0;
int g_currentBufferIndex = 0;
dispatch_semaphore_t g_inFlightSemaphore;

//MTLRenderPassDescriptor* g_mainRenderPassDesc;
id<MTLRenderCommandEncoder> g_mainPassCommandEncoder;


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

static TArray<id<MTLBuffer>, 1024> g_buffers;
static TArray<id<MTLFunction>, 1024> g_shaders;
static TArray<id<MTLTexture>, 1024> g_textures;
static TArray<MTLVertexDescriptor*, 16> g_vertexDescriptors;
static TArray<id<MTLRenderPipelineState>, 16> g_renderPipelineStates;

std::unordered_map<std::string, Shader*> g_CompiledShaders;

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA 1
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>


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


namespace FishEngine
{
	class RenderPipelineStateImpl
	{
	public:
		id<MTLRenderPipelineState> m_rps;
	};
}

	
void FishEngine::InitGraphicsAPI(GLFWwindow* window)
{
	g_inFlightSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);
	
	g_window = window;
	NSWindow* nsWindow = (NSWindow*)glfwGetCocoaWindow(window);
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
	
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("/Users/yushroom/program/FishEngine-ECS/Assets/Fonts/DroidSans.ttf", 16.0f);
	ImGui_ImplMetal_Init(g_device);
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	
	
	NSString* osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
	if ([osxMode  isEqual: @"Dark"])
		ImGui::StyleColorsDark();
	else
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
	
	PUNTVertex::StaticInit();
	
	// before shader compile
	
//	id<MTLFunction> vertexFunction = [g_defaultLibrary newFunctionWithName:@"Normal_VS"];
//	id<MTLFunction> fragmentFunction = [g_defaultLibrary newFunctionWithName:@"Normal_PS"];
	Shader* normalShader = ShaderUtil::CompileFromShaderName("pbrMetallicRoughness");
	
	RenderPipelineState rps;
	rps.SetShader(normalShader);
	rps.SetVertexDecl(PUNTVertex::ms_decl);
	rps.Create();
	
	g_pipelineState = rps.impl->m_rps;
	
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
	
	MTLTextureDescriptor* depthBufferDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:800*2 height:600*2 mipmapped:NO];
	depthBufferDesc.resourceOptions = MTLResourceStorageModePrivate;
	depthBufferDesc.usage = MTLTextureUsageRenderTarget;
	g_mainDepthBuffer = [g_device newTextureWithDescriptor:depthBufferDesc];
	g_mainDepthBuffer.label = @"Main Depth buffer";
}

void FishEngine::ResetGraphicsAPI()
{
	int fbw, fbh;
	glfwGetFramebufferSize(g_window, &fbw, &fbh);
	auto w = (NSWindow*)glfwGetCocoaWindow(g_window);
	auto view = w.contentView;
	auto layer = (CAMetalLayer*)view.layer;
	auto bounds = view.bounds;
	layer.frame = bounds;
	layer.drawableSize = CGSizeMake(fbw, fbh);
//	viewportSize.x = fbw;
//	viewportSize.y = fbh;
	
	MTLTextureDescriptor* depthBufferDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:fbw height:fbh mipmapped:NO];
	depthBufferDesc.resourceOptions = MTLResourceStorageModePrivate;
	depthBufferDesc.usage = MTLTextureUsageRenderTarget;
	g_mainDepthBuffer = [g_device newTextureWithDescriptor:depthBufferDesc];
	g_mainDepthBuffer.label = @"Main Depth buffer";
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
	auto t = camera->GetTransform();
	g_perCameraUniforms.WorldSpaceCameraPos.Set( t->GetPosition().normalized(), 1 );
	g_perCameraUniforms.WorldSpaceCameraDir.Set( t->GetForward(), 0);
}

void FishEngine::SetLight(Light* light)
{
	auto t = light->GetTransform();
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
	
	g_currentFrameDrawable = [g_metalLayer nextDrawable];
	
	g_frameNumber ++;
	g_currentBufferIndex = (g_currentBufferIndex+1) % MaxBuffersInFlight;
	
	 MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = g_currentFrameDrawable.texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 1.0, 1.0, 1.0);
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = g_mainDepthBuffer;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	
	g_mainPassCommandEncoder = [g_currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
	[g_mainPassCommandEncoder setLabel:@"MainPass"];
	//	[renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
	[g_mainPassCommandEncoder setCullMode:MTLCullModeBack];
//	MTLViewport vp = {0, 0, 1600, 1200, 0, 1};
//	[g_mainPassCommandEncoder setViewport:vp];
	[g_mainPassCommandEncoder setDepthStencilState:g_normalDepthStencilState];
	[g_mainPassCommandEncoder setRenderPipelineState:g_pipelineState];
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

void FishEngine::EndPass()
{
	[g_mainPassCommandEncoder endEncoding];
}

void FishEngine::Draw(FishEngine::Mesh* mesh, FishEngine::Material* mat, int submeshID)
{
	auto vb = mesh->m_VertexBuffer;
	auto ib = mesh->m_IndexBuffer;
	[g_mainPassCommandEncoder setVertexBuffer:g_buffers[vb.idx] offset:0 atIndex:0];
//	PerDrawUniforms uniforms;
//	uniforms.MATRIX_M = g_modelMat;
//	uniforms.MATRIX_MV = g_viewMat * g_modelMat;
//	uniforms.MATRIX_MVP = g_projMat * uniforms.MATRIX_MV;
//	uniforms.MATRIX_IT_M = g_modelMat.inverse().transpose();
//	[g_mainPassCommandEncoder setVertexBuffer:g_uniformBuffers[g_currentBufferIndex] offset:0 atIndex:1];
//	FishEngine::Vector4 u_color(1, 1, 0, 1);
	
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
			struct {
				Vector4 a;
				float b;
				float c;
				float d;
			} b;
			b.a = mat->m_MaterialProperties.vec4s["baseColorFactor"];
			b.b = mat->m_MaterialProperties.vec4s["Metallic"].x;
			b.c = mat->m_MaterialProperties.vec4s["Roughness"].x;
			b.d = mat->m_MaterialProperties.vec4s["Specular"].x;
			[g_mainPassCommandEncoder setFragmentBytes:&b length:arg.size atIndex:arg.index];
		}
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
//	[g_mainPassCommandEncoder endEncoding];
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
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	g_imguiRenderPassDesc = renderPassDescriptor;
}

void FishEngine::ImguiRender()
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	id<MTLRenderCommandEncoder> renderEncoder = [g_currentCommandBuffer renderCommandEncoderWithDescriptor:g_imguiRenderPassDesc];
	
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
	auto a = g_vertexDescriptors[m_Index].attributes[m_AttribCount];
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
	return ShaderUniformBufferType::Custom;
};


ShaderUniformSignature GetShaderUniformSignature(NSArray <MTLArgument *> *args)
{
	ShaderUniformSignature signature;
	for (MTLArgument* arg in args)
	{
		printf("arg: %s type: %lu index: %lu\n", arg.name.UTF8String, arg.type, (unsigned long)arg.index);
		
		if (arg.active)
		{
			if (arg.type == MTLArgumentTypeBuffer)
			{
				ShaderUniformBuffer ub;
				ub.name = arg.name.UTF8String;
				ub.index = arg.index;
				if (MTLDataTypeStruct == arg.bufferDataType)
				{
					printf("size: %lu\n", (unsigned long)arg.bufferDataSize);
					ub.size = arg.bufferDataSize;
				}
				
				ub.type = IsInternalShaderUniformBuffer(ub.name);
				if (ub.type == ShaderUniformBufferType::Custom)
				{
					for (MTLStructMember* uniform in arg.bufferStructType.members)
					{
						const char* name = uniform.name.UTF8String;
						printf("uniform name: %s offset: %lu type: %lu\n", name, (unsigned long)uniform.offset, (unsigned long)uniform.dataType);
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
	psd.label = @"Simple Pipeline";
	psd.vertexFunction = g_shaders[shader->m_VertexShader.idx];
	psd.fragmentFunction = g_shaders[shader->m_FragmentShader.idx];
	psd.colorAttachments[0].pixelFormat = g_metalLayer.pixelFormat;
	psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	
	psd.vertexDescriptor = g_vertexDescriptors[PUNTVertex::ms_decl.GetIndex()];
	
	NSError* error = NULL;
	
	MTLPipelineOption option = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
	MTLRenderPipelineReflection* reflection;
	id<MTLRenderPipelineState> rps = [g_device newRenderPipelineStateWithDescriptor:psd options:option reflection:&reflection error:&error];
	
	printf("vertex arguments:\n");
	shader->m_VertexShaderSignature = GetShaderUniformSignature(reflection.vertexArguments);
	printf("fragment arguments:\n");
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
	
	void RenderPipelineState::Create()
	{
		assert(impl == nullptr);
		impl = std::make_unique<RenderPipelineStateImpl>();
		MTLRenderPipelineDescriptor* psd = [[MTLRenderPipelineDescriptor alloc] init];
		psd.label = @"Simple Pipeline";
		psd.vertexFunction = g_shaders[shader->m_VertexShader.idx];
		psd.fragmentFunction = g_shaders[shader->m_FragmentShader.idx];
		psd.colorAttachments[0].pixelFormat = g_metalLayer.pixelFormat;
		psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
		
		psd.vertexDescriptor = g_vertexDescriptors[vertexDecl.GetIndex()];
		
		NSError* error = NULL;
		impl->m_rps = [g_device newRenderPipelineStateWithDescriptor:psd error:&error];
#if 0
		MTLPipelineOption option = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
		MTLRenderPipelineReflection* reflection;
		impl->m_rps = [g_device newRenderPipelineStateWithDescriptor:psd options:option reflection:&reflection error:&error];
		
		printf("vertex arguments:\n");
		vertexShaderSignature = GetShaderUniformSignature(reflection.vertexArguments);
		printf("fragment arguments:\n");
		fragmentShaderSignature = GetShaderUniformSignature(reflection.fragmentArguments);
#endif
		if (!impl->m_rps)
		{
			NSLog(@"Failed to created pipeline state, error %@", error);
			abort();
			exit(1);
		}
	}
}
