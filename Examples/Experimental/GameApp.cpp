#include "GameApp.hpp"

#include <cstdio>
#include <cstdlib>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>

using namespace FishEngine;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void OnWindowResize(GLFWwindow* window)
//{
//	int fbw, fbh;
//	glfwGetFramebufferSize(window, &fbw, &fbh);
//	auto w = (NSWindow*)glfwGetCocoaWindow(window);
//	auto view = w.contentView;
//	auto layer = (CAMetalLayer*)view.layer;
//	auto bounds = view.bounds;
//	layer.frame = bounds;
//	layer.drawableSize = CGSizeMake(fbw, fbh);
//	viewportSize.x = fbw;
//	viewportSize.y = fbh;
//}
{
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	OnWindowResize(window);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}


static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}


void GameApp::Run()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		abort();
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
	if (!m_Window)
	{
		glfwTerminate();
		abort();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowSizeCallback(m_Window, glfw_window_size_callback);
	glfwSetKeyCallback(m_Window, glfw_key_callback);
	glfwSetMouseButtonCallback(m_Window, glfw_mouse_button_callback);
	glfwSetScrollCallback(m_Window, glfw_scroll_callback);
	//glfwSetWindowIconifyCallback(g_gWindow, glfw_window_iconify_callback);
	glfwSetCharCallback(m_Window, glfw_char_callback);
	//glfwSetWindowFocusCallback(g_gWindow, glfw_window_focus_callback);

	Start();
	OnWindowResize(m_Window);

	while (!glfwWindowShouldClose(m_Window))
	{

		Update();
		glfwPollEvents();
	}

	glfwTerminate();
}