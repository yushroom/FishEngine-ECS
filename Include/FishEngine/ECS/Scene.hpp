#pragma once

#include "../Engine.hpp"
//#include "System.hpp"
#include "Component.hpp"
#include "GameObject.hpp"
#include "SingletonSystem.hpp"

#include <unordered_map>
#include <functional>

namespace FishEngine
{
	class System;
	class SingletonInput;
	class SingletonTime;
	
	class Scene
	{
		friend class RenderSystem;
	public:
		//EntityID CreateGameObject();
		GameObject* CreateGameObject();
		
		inline static Scene* s_Current = nullptr;
		
		template<class T>
		T* GameObjectAddComponent(GameObject* go)
		{
			T* comp = T::Create();
			go->m_Components.push_back(comp);
			comp->entityID = go->ID;
			comp->m_GameObject = go;
			return comp;
		}
		
		template<class T>
		T* GameObjectAddComponent(EntityID id)
		{
			return GameObjectAddComponent<T>(GetGameObjectByID(id));
		}
		
		//	void GameObjectSetParent(EntityID child, EntityID parent)
		//	{
		//		auto c = GetGameObjectByID(child);
		////		auto p = GetGameObjectByID(parent);
		//		c->m_ParentID = parent;
		//	}
		
		void All(std::function<void(GameObject*)> func)
		{
			for (auto& pair : m_GameObjects)
			{
				func(pair.second);
			}
		}
		
		template<class T>
		void ForEach(std::function<void(GameObject*, T*)> func)
		{
			for (T* t : T::components)
			{
				GameObject* go = GetGameObjectByID(t->entityID);
				func(go, t);
			}
		}
		
		
		template<class T1, class T2>
		void ForEach(std::function<void(GameObject*, T1*, T2*)> func)
		{
			for (auto& pair : m_GameObjects)
			{
				T1* t1 = nullptr;
				T2* t2 = nullptr;
				GameObject* go = pair.second;
				for (Component* comp : go->m_Components)
				{
					if (t1 != nullptr && comp->Is<T1>())
					{
						t1 = (T1*)comp;
					}
					else if (t2 != nullptr && comp->Is<T2>())
					{
						t2 = (T2*)comp;
					}
					
					if (t1 != nullptr && t2 != nullptr)
					{
						func(go, t1, t2);
						return;
					}
				}
			}
		}
		
		
		template<class T>
		T* FindComponent() const
		{
			if (T::components.empty())
				return nullptr;
			return T::components.front();
		}
		
		template<class T>
		T* AddSingletonComponent()
		{
			static_assert(std::is_base_of_v<SingletonComponent, T>);
			auto typeidx = std::type_index(typeid(T));
			auto it = m_SingletonComponents.find(typeidx);
			if (it != m_SingletonComponents.end())
			{
				abort();
			}
			
			T* t = new T();
			m_SingletonComponents[typeidx] = t;
			return t;
		}
		
		template<class T>
		T* GetSingletonComponent()
		{
			static_assert(std::is_base_of_v<SingletonComponent, T>);
			auto it = m_SingletonComponents.find(std::type_index(typeid(T)));
			if (it == m_SingletonComponents.end())
				return nullptr;
			return (T*)it->second;
		}
		
		template<class T>
		T* AddSystem()
		{
			static_assert(std::is_base_of_v<System, T>);
			T* system = new T();	// did you forget to add SYSTEM(T) ?
			m_Systems.push_back(system);
			system->m_Scene = this;
			system->OnAdded();
			return system;
		}
		
		template<class T>
		T* GetSystem();
		
		void Start();
		void Update();
		void PostUpdate();
		
		GameObject* GetGameObjectByID(EntityID id)
		{
			auto it = m_GameObjects.find(id);
			if (it == m_GameObjects.end())
				return nullptr;
			return it->second;
		}
		
		const std::vector<System*>& GetSystems() const
		{
			return m_Systems;
		}
		
		
		bool m_Cleaning = false;
		std::vector<Transform*> m_RootTransforms;
		const auto& GetRootTransforms() { return m_RootTransforms; }
		void AddRootTransform(Transform* t);
		void RemoveRootTransform(Transform* t);
		
		SingletonInput* input = nullptr;
		SingletonTime* time = nullptr;
		
	protected:
		std::unordered_map<EntityID, GameObject*> m_GameObjects;
		std::vector<System*> m_Systems;
		std::unordered_map<std::type_index, SingletonComponent*> m_SingletonComponents;
		
	private:
		EntityID m_LastEntityID = 0;
	};

}
