#pragma once

struct Vector2
{
	float x = 0;
	float y = 0;

	void Set(float x, float y) { this->x = x; this->y = y; }

	friend const Vector2& operator-(const Vector2& v1, const Vector2& v2)
	{
		return { v1.x - v2.x, v1.y - v2.y };
	}
};

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;

	void Set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }

	Vector3 operator-() const
	{
		return Vector3{ -x, -y, -z };
	}

	void operator*=(float v)
	{
		x *= v; y *= v; z *= v;
	}

	float Length() const
	{
		return sqrtf(x*x + y*y + z*z);
	}

	static Vector3 Normalize(const Vector3& v)
	{
		float invLen = 1.0f / v.Length();
		return { v.x*invLen, v.y*invLen, v.z*invLen };
	}

	static float Distance(const Vector3& a, const Vector3& b)
	{
		float x = a.x - b.x;
		float y = a.y - b.y;
		float z = a.z - b.z;
		return sqrtf(x*x + y * y + z * z);
	}
};

struct Quaternion
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};
