#pragma once

#include <FishEngine/ECS/System.hpp>

namespace FishEditor
{
	class SelectionSystem : public FishEngine::System
	{
		SYSTEM(SelectionSystem);
	public:

		void OnAdded() override;
	};
}
