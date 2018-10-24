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
		GLFWwindow* m_Window = nullptr;
	};
}