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
	std::vector<float> output;

	Quaternion SampleQuat(float time, const Quaternion& initValue);

	Vector3 SampleVector3(float time, const Vector3& initValue);

private:
	Vector3 _GetVector3(int idx);

	Quaternion _GetQuaternion(int idx);
};

class Animation : public ECS::Component
{
	COMPONENT(Animation);
	friend class ModelUtil;
public:
	std::vector<AnimationCurve> curves;
	float length = 0;
};
