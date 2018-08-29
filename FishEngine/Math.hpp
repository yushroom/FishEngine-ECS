#pragma once

struct Vector2
{
	float x = 0;
	float y = 0;
};

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;

	static Vector3 Normalize(const Vector3& v)
	{
		float invLen = 1.0f / sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		return { v.x*invLen, v.y*invLen, v.z*invLen };
	}
};

struct Quaternion
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};
