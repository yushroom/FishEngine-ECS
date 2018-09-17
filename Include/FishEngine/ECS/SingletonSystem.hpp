#pragma once

namespace FishEngine
{
	class Scene;
	
	class SingletonComponent
	{
		friend class Scene;
	protected:
		SingletonComponent() = default;
	};
	
#define SINGLETON_COMPONENT(T) \
	friend class FishEngine::Scene;

}
