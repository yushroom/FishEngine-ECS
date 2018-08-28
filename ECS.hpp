
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>
#include "Math.hpp"
#include "Engine.hpp"
#include "Object.hpp"

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
	friend class Scene;                         \
	inline static std::vector<T*> components;   \
	std::type_index GetTypeIndex() override     \
	{ return std::type_index(typeid(T)); }      \
	static T* Create() { T* t = new T(); components.push_back(t); return t; }               \



class Transform : public Component
{
	COMPONENT(Transform)
	friend class GameObject;
public:
	Vector3 position;
	Vector3 scale;
	Quaternion rotation;
};


class GameObject : public Object
{
	friend class Scene;
public:
	GameObject(EntityID entityID, Scene* scene);
	
	EntityID ID;
	Matrix transformMatrix = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
	
	Transform* GetTransform() const
	{
		return components.front()->As<Transform>();
	}
	
	EntityID GetParent() const { return parentID; }
	
protected:
	std::vector<Component*> components;
	EntityID parentID = 0;
	int rootOrder = 0;		// index in parent's children array
};


class ISystem
{
public:
	virtual void Update(Scene* scene) = 0;
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
		GameObjectAddComponent<Transform>(go);
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
	
	void AddSystem(ISystem* system)
	{
		m_Systems.push_back(system);
	}
	
	void Update()
	{
		for (ISystem* s : m_Systems)
		{
			s->Update(this);
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
	
private:
	EntityID m_LastEntityID = 0;
};

