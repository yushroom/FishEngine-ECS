#include <FishEngine/Components/Animation.hpp>

Quaternion AnimationCurve::SampleQuat(float time, const Quaternion& initValue)
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

Vector3 AnimationCurve::SampleVector3(float time, const Vector3& initValue)
{
	int count = output.size() / 3;
	if (time > input.back())
		return _GetVector3(count - 1);
	//while (time > input.back())
	//	time -= input.back();
	if (time <= input.front())
		return Vector3::Lerp(initValue, _GetVector3(0), time / input[0]);
	int i = 0;
	for (; i < input.size(); ++i)
	{
		if (time <= input[i])
			break;
	}
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
