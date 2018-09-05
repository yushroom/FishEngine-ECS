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

	Quaternion SampleQuat(float time, const Quaternion& initValue)
	{
		int count = output.size() / 3;
		if (time > input.back())
			return _GetQuaternion(count - 1);
		while (time > input.back())
			time -= input.back();
		if (time <= input.front())
			return Quaternion::Slerp(initValue, _GetQuaternion(0), time / input[0]);
		int i = 0;
		for (; i < input.size(); ++i)
		{
			if (time <= input[i])
				break;
		}
		Quaternion left = _GetQuaternion(i - 1);
		Quaternion right = _GetQuaternion(i);
		float t = time - input[i - 1];
		t /= input[i] - input[i - 1];
		return Quaternion::Slerp(left, right, t);
	}

	Vector3 SampleVector3(float time, const Vector3& initValue)
	{
		int count = output.size() / 3;
		if (time > input.back())
			return _GetVector3(count-1);
		//while (time > input.back())
		//	time -= input.back();
		if (time <= input.front())
			return Vector3::Lerp(initValue, _GetVector3(0), time/input[0]);
		int i = 0;
		for (; i < input.size(); ++i)
		{
			if (time <= input[i])
				break;
		}
		Vector3 left = _GetVector3(i-1);
		Vector3 right = _GetVector3(i);
		float t = time - input[i - 1];
		t /= input[i] - input[i - 1];
		return Vector3::Lerp(left, right, t);
	}

private:
	Vector3 _GetVector3(int idx)
	{
		float x = output[idx*3];
		float y = output[idx*3 + 1];
		float z = output[idx*3 + 2];
		return { x, y, z };
	}

	Quaternion _GetQuaternion(int idx)
	{
		float x = output[idx * 4];
		float y = output[idx * 4 + 1];
		float z = output[idx * 4 + 2];
		float w = output[idx * 4 + 3];
		return { x, y, z, w };
	}
};

class Animation : public ECS::Component
{
	COMPONENT(Animation);
	friend class ModelUtil;
public:
	std::vector<AnimationCurve> curves;
	float length = 0;
};
