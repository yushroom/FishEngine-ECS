#pragma once

#include "../ECS.hpp"


class EditorSystem : public ECS::ISystem
{
	SYSTEM(EditorSystem);
public:
	void OnAdded() override;
	void Update() override;

public:
	void MainMenu();
	void Hierarchy();
	void Inspector();
};