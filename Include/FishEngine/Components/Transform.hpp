#pragma once

#include "../ECS.hpp"

class Transform : public ECS::Component
{
	COMPONENT(Transform);
	friend class ::TransformSystem;
public:
	Vector3 position = {0, 0, 0};
	Vector3 scale = {1, 1, 1};
	Quaternion rotation;
	
	const Matrix4x4& GetLocalToWorldMatrix() const { return m_LocalToWorldMatrix; }
	
	Quaternion GetRotation() const { return m_LocalToWorldMatrix.ToRotation(); }
	Vector3 GetUp() const { return GetRotation() * Vector3::up; }
	Vector3 GetRight() const { return GetRotation() * Vector3::right; }
	Vector3 GetForward() const { return GetRotation() * Vector3::forward; }
	
	// TODO: move to TransformSystem
	
	Vector3 TransformDirection(const Vector3& direction) const
	{
		return m_LocalToWorldMatrix.MultiplyVector(direction);
	}
	
	void Translate(const Vector3& translation)
	{
		position += TransformDirection(translation);
	}
	
	void RotateAround(const Vector3& point, const Vector3& axis, float angle)
	{
		// step1: update position
		auto vector = this->position;
		auto rot = Quaternion::AngleAxis(angle, axis);
		Vector3 vector2 = vector - point;
		vector2 = rot * vector2;
		vector = point + vector2;
		position = vector;
		
		// step2: update rotation
		rotation = rot * rotation;
	}
	
protected:
	Matrix4x4 m_LocalToWorldMatrix = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
};
