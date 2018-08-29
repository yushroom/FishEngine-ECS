#pragma once
#include "Shader.hpp"

class Material : public Object
{
public:	
	Shader* m_Shader = nullptr;

	float pbrparams[4] = { 0, 0.5, 0.5, 0 };
};
