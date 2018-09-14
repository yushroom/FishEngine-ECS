#include <FishEngine/Systems/SelectionSystem.hpp>
#include <FishEngine/Components/SingletonSelection.hpp>

void SelectionSystem::OnAdded()
{
	m_Scene->AddSingletonComponent<SingletonSelection>();
}


