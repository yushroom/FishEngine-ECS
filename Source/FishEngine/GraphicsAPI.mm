#include <FishEngine/GraphicsAPI.hpp>
//#include "../../GraphicsPlatform.hpp"

#include <MetalKit/MetalKit.h>

#include <FishEngine/Mesh.hpp>
#include <FishEngine/Material.hpp>

using namespace FishEngine;

constexpr int AAPLMaxBuffersInFlight = 3;

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
NSArray<id<MTLBuffer>> *g_uniformBuffers;
id<MTLDepthStencilState> g_normalDepthStencilState;
int g_frameNumber = 0;
int g_currentBufferIndex = 0;
dispatch_semaphore_t g_inFlightSemaphore;

//MTLRenderPassDescriptor* g_mainRenderPassDesc;
id<MTLRenderCommandEncoder> g_mainPassCommandEncoder;

constexpr int MAX_BUFFER_SIZE = 1024;
int g_current_buffer_index = 0;
inline int NextBuffer() { assert(g_current_buffer_index < MAX_BUFFER_SIZE-1); return ++g_current_buffer_index; }
id<MTLBuffer> g_buffers[MAX_BUFFER_SIZE];

constexpr int MAX_SHADER_SIZE = 1024;
int g_current_shader_index = 0;
inline int NextShader() { assert(g_current_shader_index < MAX_SHADER_SIZE-1); return ++g_current_shader_index; }
id<MTLFunction> g_shaders[MAX_SHADER_SIZE];

constexpr int MAX_TEXTURE_SIZE = 1024;
int g_current_texture_index = 0;
inline int NextTexture() { assert(g_current_texture_index < MAX_TEXTURE_SIZE-1); return ++g_current_texture_index; }
id<MTLTexture> g_textures[MAX_TEXTURE_SIZE];

constexpr int MAX_VERTEX_DESC_SIZE = 16;
int g_current_vertex_desc_index = 0;
inline int NextVertexDesc() { assert(g_current_vertex_desc_index < MAX_VERTEX_DESC_SIZE-1); return ++g_current_vertex_desc_index; }
MTLVertexDescriptor* g_vertexDescriptors[MAX_VERTEX_DESC_SIZE];

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA 1
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>

struct Uniforms
{
	Matrix4x4 modelMat;
	Matrix4x4 mvpMat;
};

void FishEngine::InitGraphicsAPI(GLFWwindow* window)
{
	g_inFlightSemaphore = dispatch_semaphore_create(AAPLMaxBuffersInFlight);
	
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
	
	ImGui::StyleColorsLight();
	
	g_commandQueue = [g_device newCommandQueue];
	[g_commandQueue setLabel:@"MainCmdQueue"];
	
	g_defaultLibrary = [g_device newDefaultLibrary];
	id<MTLFunction> vertexFunction = [g_defaultLibrary newFunctionWithName:@"vs_main"];
	id<MTLFunction> fragmentFunction = [g_defaultLibrary newFunctionWithName:@"fs_main"];
	MTLRenderPipelineDescriptor* psd = [[MTLRenderPipelineDescriptor alloc] init];
	psd.label = @"Simple Pipeline";
	psd.vertexFunction = vertexFunction;
	psd.fragmentFunction = fragmentFunction;
	psd.colorAttachments[0].pixelFormat = g_metalLayer.pixelFormat;
	psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	
	int offset = 0;
	MTLVertexDescriptor* punt = [[MTLVertexDescriptor alloc] init];
	punt.attributes[0].format = MTLVertexFormatFloat3;
	punt.attributes[0].offset = 0;
	punt.attributes[0].bufferIndex = 0;
	offset += 3*4;
	
//	punt.attributes[0]. = MTLVertexFormatFloat3;
	punt.attributes[1].format = MTLVertexFormatFloat2;
	punt.attributes[1].offset = offset;
	punt.attributes[1].bufferIndex = 0;
	offset += 2*4;
	
	punt.attributes[2].format = MTLVertexFormatFloat3;
	punt.attributes[2].offset = offset;
	punt.attributes[2].bufferIndex = 0;
	offset += 3*4;
	
	punt.attributes[3].format = MTLVertexFormatFloat4;
	punt.attributes[3].offset = offset;
	punt.attributes[3].bufferIndex = 0;
	offset += 4*4;
	
	punt.layouts[0].stride = offset;
	punt.layouts[0].stepRate = 1;

	psd.vertexDescriptor = punt;
	
	NSError* error = NULL;
	g_pipelineState = [g_device newRenderPipelineStateWithDescriptor:psd error:&error];
	if (!g_pipelineState)
	{
		NSLog(@"Failed to created pipeline state, error %@", error);
		abort();
		exit(1);
	}
	
	id<MTLBuffer> uniformBuffersCArray[AAPLMaxBuffersInFlight];
	
	for (int i = 0; i < AAPLMaxBuffersInFlight; ++i)
	{
		uniformBuffersCArray[i] = [g_device newBufferWithLength:sizeof(Uniforms) options:MTLResourceStorageModeShared];
		uniformBuffersCArray[i].label = [NSString stringWithFormat:@"UniformBuffer%d", i];
	}
	
	g_uniformBuffers = [[NSArray alloc] initWithObjects:uniformBuffersCArray count:AAPLMaxBuffersInFlight];
	
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

Matrix4x4 g_modelMat;
Matrix4x4 g_viewMat;
Matrix4x4 g_projMat;


void FishEngine::SetModelMatrix(const Matrix4x4& model)
{
	g_modelMat = model;
}

void FishEngine::SetViewProjectionMatrix(const Matrix4x4& view, const Matrix4x4& proj)
{
	g_viewMat = view;
	g_projMat = proj;
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
	g_currentBufferIndex = (g_currentBufferIndex+1) % AAPLMaxBuffersInFlight;
	
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
	[g_mainPassCommandEncoder setCullMode:MTLCullModeNone];
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

void FishEngine::Draw(FishEngine::Mesh* mesh, FishEngine::Material* mat)
{
	auto vb = mesh->m_VertexBuffer;
	auto ib = mesh->m_IndexBuffer;
//	id<MTLBuffer> vb = g_buffers[vb.idx];
	[g_mainPassCommandEncoder setVertexBuffer:g_buffers[vb.idx] offset:0 atIndex:0];
	Uniforms uniforms;
	uniforms.modelMat = g_modelMat.transpose();
	uniforms.mvpMat = (g_projMat * g_viewMat * g_modelMat).transpose();
//	Uniforms* uniforms = (Uniforms*)g_uniformBuffers[g_currentBufferIndex].contents;
//	uniforms->modelMat = g_modelMat;
//	uniforms->mvpMat = g_projMat * g_viewMat * g_modelMat;
//	uniforms->modelMat = uniforms->modelMat.transpose();
//	uniforms->mvpMat = uniforms->mvpMat.transpose();
//	[g_mainPassCommandEncoder setVertexBuffer:g_uniformBuffers[g_currentBufferIndex] offset:0 atIndex:1];
	[g_mainPassCommandEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:1];
	[g_mainPassCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
							  indexCount:mesh->m_TriangleCount*3
							   indexType:MTLIndexTypeUInt32
							 indexBuffer:g_buffers[ib.idx]
					   indexBufferOffset:0];
	
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
	int size = decl.GetVertexSize();
	assert(data.byteSize % size == 0);
//	id<MTLBuffer> buffer = [g_device newBufferWithBytes:data.data
//												 length:data.byteSize
//												options:MTLResourceStorageModePrivate];
	id<MTLBuffer> buffer = [g_device newBufferWithLength:data.byteSize options:MTLResourceStorageModeShared];
	memcpy(buffer.contents, data.data, data.byteSize);
	VertexBufferHandle handle;
	handle.idx = NextBuffer();
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
	handle.idx = NextBuffer();
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

void FishEngine::VertexDecl::Begin()
{
	m_Index = NextVertexDesc();
	MTLVertexDescriptor* desc = [[MTLVertexDescriptor alloc] init];
	g_vertexDescriptors[m_Index] = desc;
}

void FishEngine::VertexDecl::Add(VertexAttrib attrib, int count, VertexAttribType type)
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

	//a.format = MTLVertexFormatFloat2;
	a.offset = m_Stride;
	a.bufferIndex = static_cast<int>(attrib);

	m_Stride += count*size;
	m_AttribCount++;
}

void FishEngine::VertexDecl::End()
{
	assert(m_Index != 0);

	auto desc = g_vertexDescriptors[m_Index];
	desc.layouts[0].stride = m_Stride;
	//desc.layouts[0].stepRate = 1;
	m_Valid = true;
}


namespace FishEngine
{
	ShaderHandle CreateShader(const char* functionName)
	{
		NSString* s = [NSString stringWithUTF8String:functionName];
		id<MTLFunction> f = [g_defaultLibrary newFunctionWithName:s];
		
		assert(f != nil);
		
		ShaderHandle h;
		h.idx = NextShader();
		g_shaders[h.idx] = f;
		return h;
	}
}
