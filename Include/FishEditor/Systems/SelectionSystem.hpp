#pragma once

#include <FishEngine/ECS.hpp>

namespace FishEditor
{
	class SelectionSystem : public FishEngine::ISystem
	{
		SYSTEM(SelectionSystem);
	public:

		void OnAdded() override;
	};
}
