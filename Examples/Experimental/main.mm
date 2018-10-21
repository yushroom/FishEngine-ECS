#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA 1
#include <GLFW/glfw3native.h>
#include <stdlib.h>
#include <stdio.h>

#include <thread>

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_metal.h>

#include "AAPLShaderTypes.h"

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

vector_int2 viewportSize;

static void OnWindowResize(GLFWwindow* window)
{
	int fbw, fbh;
	glfwGetFramebufferSize(window, &fbw, &fbh);
	auto w = (NSWindow*)glfwGetCocoaWindow(window);
	auto view = w.contentView;
	auto layer = (CAMetalLayer*)view.layer;
	auto bounds = view.bounds;
	layer.frame = bounds;
	layer.drawableSize = CGSizeMake(fbw, fbh);
	viewportSize.x = fbw;
	viewportSize.y = fbh;
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	OnWindowResize(window);
}

#define Use_MTKView 0

int main()
{
	glfwSetErrorCallback(error_callback);
	GLFWwindow* window = nullptr;
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, glfw_window_size_callback);

	NSWindow* nsWindow = (NSWindow*)glfwGetCocoaWindow(window);
#if USE_MTKView
	MTKView* view = nsWindow.contentView;
	view.device = MTLCreateSystemDefaultDevice();
	auto m_device = view.device;
#else
	NSView* view = nsWindow.contentView;
	[view setWantsLayer:YES];
	auto m_metalLayer = [CAMetalLayer layer];
	[view setLayer:m_metalLayer];
	auto m_device = MTLCreateSystemDefaultDevice();
	m_metalLayer.device = m_device;
	m_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	m_metalLayer.magnificationFilter = kCAFilterNearest;
	auto rect = view.bounds;
	m_metalLayer.frame = rect;
#endif
	
	OnWindowResize(window);

	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("/Users/yushroom/program/FishEngine-ECS/Assets/Fonts/DroidSans.ttf", 16.0f);
	ImGui_ImplMetal_Init(m_device);
//	ImGui_ImplOSX_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	
	ImGui::StyleColorsDark();
	
	id<MTLCommandQueue> commandQueue = [m_device newCommandQueue];
#if !Use_MTKView
	id<CAMetalDrawable> frameDrawable;
#endif
	id<MTLRenderPipelineState> pipelineState;
	
	
	id<MTLLibrary> defaultLibrary = [m_device newDefaultLibrary];
	id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
	id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];
	
	MTLRenderPipelineDescriptor* psd = [[MTLRenderPipelineDescriptor alloc] init];
	psd.label = @"Simple Pipeline";
	psd.vertexFunction = vertexFunction;
	psd.fragmentFunction = fragmentFunction;
	psd.colorAttachments[0].pixelFormat = m_metalLayer.pixelFormat;
	
	NSError *error = NULL;
	pipelineState = [m_device newRenderPipelineStateWithDescriptor:psd error:&error];
	if (!pipelineState)
	{
		// Pipeline State creation could fail if we haven't properly set up our pipeline descriptor.
		//  If the Metal API validation is enabled, we can find out more information about what
		//  went wrong.  (Metal API validation is enabled by default when a debug build is run
		//  from Xcode)
		NSLog(@"Failed to created pipeline state, error %@", error);
		return 1;
	}
	
	
	while (!glfwWindowShouldClose(window))
	{
//		static int count = 0;
//		printf("frame: %d\n", count);
//		count ++;
		{
			using namespace std::chrono_literals;
			//std::this_thread::sleep_for(16.67ms);
		}
		
		static const AAPLVertex triangleVertices[] =
		{
			// 2D positions,    RGBA colors
			{ {  250,  -250 }, { 1, 0, 0, 1 } },
			{ { -250,  -250 }, { 0, 1, 0, 1 } },
			{ {    0,   250 }, { 0, 0, 1, 1 } },
		};
		
		
#if Use_MTKView
		MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
#else
		frameDrawable = [m_metalLayer nextDrawable];
		MTLRenderPassDescriptor * renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = frameDrawable.texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 1.0, 1.0, 1.0);
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
#endif
		
		id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
		commandBuffer.label = @"MyCommand";
		
		id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		renderEncoder.label = @"MyRenderEncoder";
		
		[renderEncoder pushDebugGroup:@"ImGui demo"];
		
		[renderEncoder setViewport:(MTLViewport){0.0, 0.0, static_cast<double>(viewportSize.x), static_cast<double>(viewportSize.y), -1.0, 1.0}];
		[renderEncoder setRenderPipelineState:pipelineState];
		[renderEncoder setVertexBytes:triangleVertices
							   length:sizeof(triangleVertices)
							  atIndex:AAPLVertexInputIndexVertices];
		
		[renderEncoder setVertexBytes:&viewportSize
							   length:sizeof(viewportSize)
							  atIndex:AAPLVertexInputIndexViewportSize];
		
		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
						  vertexStart:0
						  vertexCount:3];
		
		
		ImGui_ImplMetal_NewFrame(renderPassDescriptor);
//		ImGui_ImplOSX_NewFrame(nsWindow.contentView);
		ImGui_ImplGlfw_NewFrame();
		
		ImGui::NewFrame();
		
		ImGui::ShowDemoWindow();
		
		ImGui::Begin("Hello");
		ImGui::Text("hhhhhhhhhhh");
		ImGui::End();
		
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplMetal_RenderDrawData(drawData, commandBuffer, renderEncoder);
		
		[renderEncoder popDebugGroup];
		[renderEncoder endEncoding];
		
#if Use_MTKView
		[commandBuffer presentDrawable: view.currentDrawable];
#else
		[commandBuffer presentDrawable: frameDrawable];
#endif
		[commandBuffer commit];
		
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
