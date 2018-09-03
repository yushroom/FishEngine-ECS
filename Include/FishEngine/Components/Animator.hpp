#pragma once

#include "../ECS.hpp"

enum class AnimationCurveType
{
	Translation,
	Rotation,
	Scale,
	Weights
};

struct AnimationCurve
{
	AnimationCurveType type;
	ECS::GameObject* node = nullptr;
	std::vector<float> input;
//	std::vector<Vector3> output;
	std::vector<float> output;
};

class Animator : public ECS::Component
{
	COMPONENT(Animator);
	friend class ModelUtil;
public:
	std::vector<AnimationCurve> curves;
};
