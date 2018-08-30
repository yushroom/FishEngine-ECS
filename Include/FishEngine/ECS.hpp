#pragma once
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>
#include "Math.hpp"
#include "Engine.hpp"
#include "Object.hpp"

class TransformSystem;

namespace ECS
{

class Scene;

using EntityID = uint32_t;

class Component
{
public:
	EntityID entityID;
	
	virtual std::type_index GetTypeIndex() = 0;
	
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
	
protected:
	Component() = default;
};


#define COMPONENT(T)                            \
protected:                                      \
	T() = default;                              \
private:                                        \
	friend class ECS::Scene;                    \
	inline static std::vector<T*> components;   \
	std::type_index GetTypeIndex() override     \
	{ return std::type_index(typeid(T)); }      \
	static T* Create() { T* t = new T(); components.push_back(t); return t; }               \



class Transform : public Component
{
	COMPONENT(Transform);
	friend class ::TransformSystem;
public:
	Vector3 position = {0, 0, 0};
	Vector3 scale = {1, 1, 1};
	Quaternion rotation;

	const Matrix4x4& GetLocalToWorldMatrix() const { return m_LocalToWorldMatrix; }

protected:
	Matrix4x4 m_LocalToWorldMatrix = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
};


class GameObject : public Object
{
	friend class Scene;
public:

	EntityID GetID() const { return ID; }
	
	Transform* GetTransform() const { return m_Transform; }
	
	EntityID GetParentID() const { return parentID; }
	
protected:
	GameObject(EntityID entityID, Scene* scene);

protected:
	std::vector<Component*> components;
	Transform* m_Transform = nullptr;
	EntityID parentID = 0;
	int rootOrder = 0;		// index in parent's children array
private:
	EntityID ID;
};


class ISystem
{
	friend Scene;
public:
	virtual void OnAdded() {}
	virtual void Start() {}
	virtual void Update() = 0;
	virtual void PostUpdate() {};

protected:
	Scene * m_Scene = nullptr;
};


class SingletonComponent
{
	friend Scene;
protected:
	SingletonComponent() = default;
};


class Scene
{
public:
	EntityID CreateGameObject()
	{
		m_LastEntityID++;
		EntityID id = m_LastEntityID;
		GameObject* go = new GameObject(id, this);
		m_GameObjects[id] = go;
		go->m_Transform = GameObjectAddComponent<Transform>(go);
		return id;
	}
	
	template<class T>
	T* GameObjectAddComponent(GameObject* go)
	{
		T* comp = T::Create();
		go->components.push_back(comp);
		comp->entityID = go->ID;
		return comp;
	}
	
	template<class T>
	T* GameObjectAddComponent(EntityID id)
	{
		T* comp = T::Create();
		auto go = m_GameObjects[id];
		go->components.push_back(comp);
		comp->entityID = id;
		return comp;
	}
	
	void GameObjectSetParent(EntityID child, EntityID parent)
	{
		auto c = GetGameObjectByID(child);
//		auto p = GetGameObjectByID(parent);
		c->parentID = parent;
	}
	
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
		for (auto& pair : m_GameObjects)
		{
			T* t = nullptr;
			GameObject* go = pair.second;
			for (Component* comp : go->components)
			{
				if (t == nullptr)
					t = comp->As<T>();
			}
			if (t != nullptr)
			{
				func(go, t);
			}
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
			for (Component* comp : go->components)
			{
				if (comp->Is<T1>())
				{
					t1 = (T1*)comp;
				}
				else if (comp->Is<T2>())
				{
					t2 = (T2*)comp;
				}
			}
			if (t1 != nullptr && t2 != nullptr)
			{
				func(go, t1, t2);
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
		auto it = m_SingletonComponents.find(std::type_index(typeid(T)));
		if (it == m_SingletonComponents.end())
			return nullptr;
		return (T*)it->second;
	}
	
	void AddSystem(ISystem* system)
	{
		m_Systems.push_back(system);
		system->m_Scene = this;
		system->OnAdded();
	}

	template<class T>
	T* GetSystem()
	{
		for (ISystem* s : m_Systems)
		{
			T* t = dynamic_cast<T*>(s);
			if (t != nullptr)
				return t;
		}
		return nullptr;
	}

	void Start()
	{
		for (ISystem* s : m_Systems)
		{
			s->Start();
		}
	}
	
	void Update()
	{
		for (ISystem* s : m_Systems)
		{
			s->Update();
		}
	}

	void PostUpdate()
	{
		for (ISystem* s : m_Systems)
		{
			s->PostUpdate();
		}
	}
	
	GameObject* GetGameObjectByID(EntityID id)
	{
		auto it = m_GameObjects.find(id);
		if (it == m_GameObjects.end())
			return nullptr;
		return it->second;
	}
	
protected:
	std::unordered_map<EntityID, GameObject*> m_GameObjects;
	std::vector<ISystem*> m_Systems;
	std::unordered_map<std::type_index, SingletonComponent*> m_SingletonComponents;
	
private:
	EntityID m_LastEntityID = 0;
};

} // namespace ECS
