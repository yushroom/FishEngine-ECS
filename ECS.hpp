
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>


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


class GameObject : public Object
{
public:
	
	GameObject(EntityID entityID) : ID(entityID) { }
	
	EntityID ID;
	std::vector<Component*> components;
	Matrix transform;
};


class ISystem
{
public:
	virtual void Update(Scene* scene) = 0;
};


class Scene
{
public:
	std::unordered_map<EntityID, GameObject*> gameObjects;
	std::vector<ISystem*> systems;
	
	EntityID CreateGameObject()
	{
		lastEntityID++;
		EntityID id = lastEntityID;
		GameObject* go = new GameObject(id);
		gameObjects[id] = go;
		return id;
	}
	
	template<class T>
	T* GameObjectAddComponent(EntityID id)
	{
		T* comp = T::Create();
		auto go = gameObjects[id];
		go->components.push_back(comp);
		return comp;
	}
	
	template<class T>
	void ForEach(std::function<void(GameObject*, T*)> func)
	{
		for (auto& pair : gameObjects)
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
		for (auto& pair : gameObjects)
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
	T* FindComponent()
	{
		if (T::components.empty())
			return nullptr;
		return T::components.front();
	}
	
private:
	EntityID lastEntityID = 0;
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
