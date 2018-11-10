#pragma once

#include "GameApp.hpp"

struct GLFWwindow;

namespace FishEditor
{
	class GLFWGameApp : public GameApp
	{
	public:
		void Init() override;
		void Update() override;
		void CleanUp() override;
		
	protected:
		GLFWwindow * 	m_Window 		= nullptr;
	};
}
