#pragma once
#include "Shader.hpp"

class Material : public Object
{
public:	
	Shader* m_Shader = nullptr;
};
