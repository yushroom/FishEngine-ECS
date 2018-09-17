#include <FishEditor/Systems/SelectionSystem.hpp>
#include <FishEditor/Components/SingletonSelection.hpp>

using namespace FishEditor;

void SelectionSystem::OnAdded()
{
	m_Scene->AddSingletonComponent<SingletonSelection>();
}


