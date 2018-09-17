#pragma once

#include <typeindex>
#include "GameObject.hpp"
#include "Scene.hpp"

namespace FishEngine
{
	class Scene;
	
	class System
	{
		friend class Scene;
	public:
		virtual void OnAdded() {}
		virtual void Start() {}
		virtual void Update() {}
		virtual void PostUpdate() {}
//		virtual void Draw() {}
//		virtual void DrawOverlay() {}
		
		int m_Priority = 0;
		
		virtual std::type_index GetTypeIndex() = 0;
		
		constexpr static const char* CLASS_NAME = "System";
		virtual const char* GetClassName() { return CLASS_NAME; }
		
		template<class T>
		bool Is() const
		{
			const T* t = dynamic_cast<const T*>(this);
			return t != nullptr;
		}
		
		template<class T>
		T* As()
		{
			return dynamic_cast<T*>(this);
		}
		
		
		bool m_Enabled = true;
		
	protected:
		Scene * m_Scene = nullptr;
		
	};
	
#define SYSTEM(T)                            	\
	protected:                                      \
	T() = default;                              \
	private:                                        \
	friend class FishEngine::Scene;                    \
	std::type_index GetTypeIndex() override { return std::type_index(typeid(T)); }	\
	constexpr static const char* CLASS_NAME = #T;									\
	const char* GetClassName() override { return CLASS_NAME; }						\
	static T* Create() { T* t = new T(); return t; }								\

	
	template<class T>
	T* Scene::GetSystem()
	{
		static_assert(std::is_base_of_v<System, T>);
		for (System* s : m_Systems)
		{
			//T* t = dynamic_cast<T*>(s);
			T* t = s->As<T>();
			if (t != nullptr)
				return t;
		}
		return nullptr;
	}

}
