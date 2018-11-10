#include <FishEditor/GLFWGameApp.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/Screen.hpp>
#include <FishEngine/Texture.hpp>
#include <FishEngine/Material.hpp>
#include <FishEngine/Gizmos.hpp>
#include <FishEngine/Render/RenderViewType.hpp>
#include <FishEngine/Systems/TransformSystem.hpp>
#include <FishEngine/Systems/RenderSystem.hpp>
#include <FishEngine/Systems/InputSystem.hpp>
#include <FishEngine/Components/SingletonTime.hpp>

#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Systems/DrawGizmosSystem.hpp>
#include <FishEditor/Systems/EditorSystem.hpp>
#include <FishEditor/Systems/SceneViewSystem.hpp>
#include <FishEngine/GraphicsAPI.hpp>

#include <FishEditor/Systems/FreeCameraSystem.hpp>

#if USE_GLFW
#	include <GLFW/glfw3.h>
#	include <imgui.h>
#	include <imgui_impl_glfw.h>
#else
#	include <imgui.h>
#endif

#include <thread>

using namespace FishEditor;
using namespace FishEngine;

extern GameApp* mainApp;

#if USE_GLFW

extern GLFWwindow* g_window;


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	mainApp->Resize(width, height);
	int fbw = width;
	int fbh = height;
	glfwGetFramebufferSize(window, &fbw, &fbh);
	FishEngine::ResetGraphicsAPI(fbw, fbh);
}

inline KeyCode KKK(KeyCode key, int offset)
{
	return KeyCode(int(key) + offset);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (key == GLFW_KEY_UNKNOWN)
	return;
	
	KeyEvent e;
	if (action == GLFW_PRESS)
	e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
	e.action = KeyAction::Released;
	else if (action == GLFW_REPEAT)
	e.action = KeyAction::Held;
	else
	return;
	
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();
	
	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
	e.key = KeyCode(key);
	else if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
	e.key = KKK(KeyCode::A, key - GLFW_KEY_A);
	else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F15)
	e.key = KKK(KeyCode::F1, key - GLFW_KEY_F1);
	else if (key == GLFW_KEY_ESCAPE)
	e.key = KeyCode::Escape;
	else if (key == GLFW_KEY_LEFT_ALT)
	e.key = KeyCode::LeftAlt;
	else if (key == GLFW_KEY_RIGHT_ALT)
	e.key = KeyCode::RightAlt;
	else if (key == GLFW_KEY_LEFT_CONTROL)
	e.key = KeyCode::LeftControl;
	else if (key == GLFW_KEY_RIGHT_CONTROL)
	e.key = KeyCode::RightControl;
	else if (key == GLFW_KEY_LEFT_SUPER)
	e.key = KeyCode::LeftCommand;
	else if (key == GLFW_KEY_RIGHT_SUPER)
	e.key = KeyCode::RightCommand;
	else if (key == GLFW_KEY_LEFT)
	e.key = KeyCode::LeftArrow;
	else if (key == GLFW_KEY_RIGHT)
	e.key = KeyCode::RightArrow;
	else if (key == GLFW_KEY_UP)
	e.key = KeyCode::UpArrow;
	else if (key == GLFW_KEY_DOWN)
	e.key = KeyCode::DownArrow;
	s->PostKeyEvent(e);
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();
	KeyEvent e;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	e.key = KeyCode::MouseLeftButton;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	e.key = KeyCode::MouseRightButton;
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	e.key = KeyCode::MouseMiddleButton;
	
	if (action == GLFW_PRESS)
	e.action = KeyAction::Pressed;
	else if (action == GLFW_RELEASE)
	e.action = KeyAction::Released;
	
	s->PostKeyEvent(e);
	
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto s = mainApp->m_EditorScene->GetSystem<InputSystem>();
	s->UpdateAxis(Axis::MouseScrollWheel, (float)yoffset);
	
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void glfw_window_iconify_callback(GLFWwindow* window, int iconified)
{
	mainApp->m_WindowMinimized = (iconified == GLFW_TRUE);
}

void glfw_window_focus_callback(GLFWwindow* window, int focused)
{
	mainApp->m_WindowMinimized = (focused != GLFW_TRUE);
}



void GLFWGameApp::Init()
{
	glfwSetErrorCallback(glfw_error_callback);
	/* Initialize the library */
	if (!glfwInit())
	{
		abort();
		return;
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	/* Create a windowed mode window and its OpenGL context */
	m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "FishEngine", NULL, NULL);
	if (!m_Window)
	{
		glfwTerminate();
		abort();
		return;
	}
	
	
	/* Make the window's context current */
	//	glfwMakeContextCurrent(m_Window);
	
	//bgfx::glfwSetWindow(window);
	//	glfwSetWindow(m_Window);
	
	glfwSetWindowSizeCallback(m_Window, glfw_window_size_callback);
	glfwSetKeyCallback(m_Window, glfw_key_callback);
	glfwSetMouseButtonCallback(m_Window, glfw_mouse_button_callback);
	glfwSetScrollCallback(m_Window, glfw_scroll_callback);
	glfwSetWindowIconifyCallback(m_Window, glfw_window_iconify_callback);
	glfwSetCharCallback(m_Window, glfw_char_callback);
	glfwSetWindowFocusCallback(m_Window, glfw_window_focus_callback);
	
	glfwSetWindowSizeLimits(m_Window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
	
	
//	FishEngine::InitGraphicsAPI(m_Window);
	g_window = m_Window;
	GameApp::Init();
	
	
	//	Resize(m_WindowWidth, m_WindowHeight);
	glfw_window_size_callback(m_Window, m_WindowWidth, m_WindowHeight);
}


void GLFWGameApp::Update()
{
	if (glfwWindowShouldClose(m_Window))
	{
		m_GameOver = true;
		return;
	}
	
	auto si = m_EditorScene->input;
	if (si->IsButtonPressed(KeyCode::Escape))
		glfwSetWindowShouldClose(m_Window, 1);
	
	double cursor_x = 0, cursor_y = 0;
	glfwGetCursorPos(m_Window, &cursor_x, &cursor_y);
	cursor_x /= m_WindowWidth;
	cursor_y /= m_WindowHeight;
	m_EditorScene->GetSystem<InputSystem>()->SetMousePosition((float)cursor_x, 1.0f-(float)cursor_y);
	
	GameApp::Update();
	
	/* Poll for and process events */
	glfwPollEvents();
//		printf("============ frame end ===========\n");
}

void GLFWGameApp::CleanUp()
{
	glfwTerminate();
}

//void GLFWGameApp::Run()
//{
//	Init();
//	Start();
//
//	m_Scene->Start();
//	m_EditorScene->Start();
//
//	int frameCount = 0;
//	double timeStamp = glfwGetTime();
//
//	while (!glfwWindowShouldClose(m_Window))
//	{
//		double now = glfwGetTime();
//		m_Scene->time->deltaTime = float(now - timeStamp);
////		printf("deltaTime: %f\n", m_Scene->time->deltaTime);
//		m_EditorScene->time->deltaTime = m_Scene->time->deltaTime;
//		timeStamp = now;
//
//		Update();
//	}
//
//	/* Loop until the user closes the window */
//	glfwTerminate();
//}

#endif
