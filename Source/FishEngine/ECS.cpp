#include <FishEngine/ECS/Component.hpp>
#include <FishEngine/ECS/GameObject.hpp>
#include <FishEngine/ECS/Scene.hpp>
#include <FishEngine/ECS/System.hpp>
#include <FishEngine/Components/Transform.hpp>

using namespace FishEngine;

GameObject::GameObject(EntityID entityID, Scene* scene) : ID(entityID)
{
}

void Scene::Start()
{
	std::sort(m_Systems.begin(), m_Systems.end(), [](System* a, System* b) {
		return a->m_Priority < b->m_Priority;
	});
	
	for (System* s : m_Systems)
	{
		s->Start();
	}
}

void Scene::Update()
{
	for (System* s : m_Systems)
	{
		if (s->m_Enabled)
			s->Update();
	}
}

void Scene::PostUpdate()
{
	for (System* s : m_Systems)
	{
		s->PostUpdate();
	}
}



GameObject* Scene::CreateGameObject()
{
	m_LastEntityID++;
	EntityID id = m_LastEntityID;
	GameObject* go = new GameObject(id, this);
	m_GameObjects[id] = go;
	go->m_Transform = GameObjectAddComponent<Transform>(go);
	go->m_Scene = this;
	m_RootTransforms.push_back(go->GetTransform());
	return go;
}

void Scene::AddRootTransform(Transform* t)
{
	if (m_Cleaning)
		return;
	m_RootTransforms.push_back(t);
	//		t->m_RootOrder = m_RootTransforms.size() - 1;
}

void Scene::RemoveRootTransform(Transform* t)
{
	auto pos = std::find(m_RootTransforms.begin(), m_RootTransforms.end(), t);
	//assert(pos != m_RootTransforms.end());
	if (pos == m_RootTransforms.end()) {
		//LogWarning("transform not found");
		return;
	}
	m_RootTransforms.erase(pos);
}

