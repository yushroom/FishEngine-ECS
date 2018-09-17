#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>
#include <FishEngine/ECS/Scene.hpp>

using namespace FishEditor;

void SelectionSystem::OnAdded()
{
	m_Scene->AddSingletonComponent<SingletonSelection>();
}


