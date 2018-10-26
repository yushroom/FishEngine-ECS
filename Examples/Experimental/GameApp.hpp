#pragma once

struct GLFWwindow;

namespace FishEngine
{
	class GameApp
	{
	public:
		void Run();
		//void Init();
		virtual void Start() = 0;
		virtual void Update() = 0;
		virtual void Clean() = 0;

	protected:

		int m_WindowWidth = 800;
		int m_WindowHeight = 600;
		GLFWwindow* m_Window = nullptr;
	};
}