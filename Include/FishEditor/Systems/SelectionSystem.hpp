#pragma once

#include <FishEngine/ECS.hpp>

class SelectionSystem : public ECS::ISystem
{
	SYSTEM(SelectionSystem);
public:

	void OnAdded() override;
};
