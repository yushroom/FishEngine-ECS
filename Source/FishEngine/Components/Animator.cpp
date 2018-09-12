#include <FishEngine/Components/Animator.hpp>

Quaternion AnimationCurve::SampleQuat(float time, const Quaternion& initValue)
{
	if (input.empty())
		return initValue;
	int count = input.size();
	if (time >= input.back())
		return _GetQuaternion(count - 1);
	if (time < input.front())
		return initValue;
	if (time == input.front())
		return _GetQuaternion(0);
	int i = 1;
	for (; i < input.size(); ++i)
	{
		if (time <= input[i])
			break;
	}
	assert(i - 1 >= 0 && i < count);
	Quaternion left = _GetQuaternion(i - 1);
	Quaternion right = _GetQuaternion(i);
	float t = time - input[i - 1];
	t /= input[i] - input[i - 1];
	return Quaternion::Slerp(left, right, t);
}

Vector3 AnimationCurve::SampleVector3(float time, const Vector3& initValue)
{
	if (input.empty())
		return initValue;
	int count = input.size();
	if (time >= input.back())
		return _GetVector3(count - 1);
	if (time < input.front())
		return initValue;
	if (time == input.front())
		return _GetVector3(0);
	int i = 1;
	for (; i < input.size(); ++i)
	{
		if (time <= input[i])
			break;
	}
	assert(i - 1 >= 0 && i < count);
	Vector3 left = _GetVector3(i - 1);
	Vector3 right = _GetVector3(i);
	float t = time - input[i - 1];
	t /= input[i] - input[i - 1];
	return Vector3::Lerp(left, right, t);
}

Vector3 AnimationCurve::_GetVector3(int idx)
{
	float x = output[idx * 3];
	float y = output[idx * 3 + 1];
	float z = output[idx * 3 + 2];
	return { x, y, z };
}

Quaternion AnimationCurve::_GetQuaternion(int idx)
{
	float x = output[idx * 4];
	float y = output[idx * 4 + 1];
	float z = output[idx * 4 + 2];
	float w = output[idx * 4 + 3];
	return { x, y, z, w };
}
