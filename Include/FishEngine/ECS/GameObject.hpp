#pragma once

#include "../Object.hpp"
#include "Component.hpp"

namespace FishEngine
{
	class Component;
	class Transform;
	
	class GameObject : public NamedObject
	{
		friend class Scene;
	public:
		
		EntityID GetID() const { return ID; }
		
		Transform* GetTransform() const { return m_Transform; }
		//EntityID GetParentID() const { return m_ParentID; }
		
		const std::vector<Component*>& GetComponents() const { return m_Components; }
		
		template<class T>
		T* GetComponent() {
			for (auto comp : m_Components)
			{
				T* t = comp->As<T>();
				if (t != nullptr)
					return t;
			}
			return nullptr;
		}
		
		Scene* GetScene() { return m_Scene; }
		
//		std::string name;
		
	protected:
		GameObject(EntityID entityID, Scene* scene);
		
	protected:
		std::vector<Component*> m_Components;
		Transform* m_Transform = nullptr;
		//EntityID m_ParentID = 0;
		//int m_RootOrder = 0;		// index in parent's children array
		Scene* m_Scene = nullptr;
	private:
		EntityID ID;
	};
}
