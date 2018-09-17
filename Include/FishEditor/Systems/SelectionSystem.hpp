#pragma once

#include <FishEngine/ECS.hpp>

namespace FishEditor
{
	class SelectionSystem : public FishEngine::System
	{
		SYSTEM(SelectionSystem);
	public:

		void OnAdded() override;
	};
}
