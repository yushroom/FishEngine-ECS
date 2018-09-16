#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>

void SelectionSystem::OnAdded()
{
	m_Scene->AddSingletonComponent<SingletonSelection>();
}


