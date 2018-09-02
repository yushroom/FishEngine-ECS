#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>

using namespace ECS;

GameObject::GameObject(EntityID entityID, Scene* scene) : ID(entityID)
{
}

EntityID Scene::CreateGameObject()
{
	m_LastEntityID++;
	EntityID id = m_LastEntityID;
	GameObject* go = new GameObject(id, this);
	m_GameObjects[id] = go;
	go->m_Transform = GameObjectAddComponent<Transform>(go);
	return id;
}
