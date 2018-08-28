#pragma once

#include "ClassDef.hpp"
#include "Engine.hpp"

class System : public Singleton
{
public:
	virtual void Init() {}
	virtual void Start() {}
	virtual void Update(float deltaTime) {}
	virtual void Clean() {}
};


class RenderSystem : public System
{
public:
	
	static RenderSystem& GetInstance()
	{
		static RenderSystem instance;
		return instance;
	}
	
	void Init2(int width, int height);
	
	virtual void Start() override
	{
		
	}
	
	virtual void Update(float deltaTime) override
	{
		
	}
	
	virtual void Clean() override
	{
		
	}
	
	uint64_t GetState() { return s_State; }
	
	
private:
	uint64_t s_State = 0;
};
