#pragma once

#include "../ECS.hpp"

enum class Space
{
	World,
	Self
};

class Transform : public ECS::Component
{
	COMPONENT(Transform);
	friend class ::TransformSystem;
public:
	virtual ~Transform() = default;

	// The position of the transform in world space.
	Vector3 GetPosition() const
	{
		UpdateMatrix();
		//return m_localToWorldMatrix.MultiplyPoint(0, 0, 0);
		auto& l2w = m_LocalToWorldMatrix;
		return Vector3(l2w.m[0][3], l2w.m[1][3], l2w.m[2][3]);
	}

	// position is in world space
	void SetPosition(const Vector3& position)
	{
		SetPosition(position.x, position.y, position.z);
	}

	void SetPosition(const float x, const float y, const float z)
	{
		if (m_Father != nullptr)
		{
			m_LocalPosition = m_Father->GetWorldToLocalMatrix()._MultiplyPoint(x, y, z);
		}
		else
		{
			m_LocalPosition.Set(x, y, z);
		}
		MakeDirty();
	}


	// Position of the transform relative to the parent transform.
	Vector3 GetLocalPosition() const
	{
		return m_LocalPosition;
	}

	void SetLocalPosition(const Vector3& position)
	{
		m_LocalPosition = position;
		MakeDirty();
	}

	void SetLocalPosition(const float x, const float y, const float z)
	{
		m_LocalPosition.Set(x, y, z);
		MakeDirty();
	}


	// The rotation as Euler angles in degrees.
	Vector3 GetEulerAngles() const
	{
		return GetRotation().eulerAngles();
	}

	void SetEulerAngles(const Vector3& eulerAngles)
	{
		this->SetRotation(Quaternion::Euler(eulerAngles));
	}


	// The rotation as Euler angles in degrees relative to the parent transform's rotation.
	Vector3 GetLocalEulerAngles() const
	{
		return m_LocalRotation.eulerAngles();
	}

	void SetLocalEulerAngles(const Vector3& eulerAngles)
	{
		m_LocalRotation.setEulerAngles(eulerAngles);
		MakeDirty();
	}

	void SetLocalEulerAngles(const float x, const float y, const float z)
	{
		SetLocalEulerAngles(Vector3(x, y, z));
	}


	// The rotation of the transform in world space stored as a Quaternion.
	Quaternion GetRotation() const
	{
		UpdateMatrix();
		return m_LocalToWorldMatrix.ToRotation();
	}

	void SetRotation(const Quaternion& new_rotation)
	{
		if (m_Father == nullptr)
		{
			m_LocalRotation = new_rotation;
		}
		else
		{
			m_LocalRotation = Quaternion::Inverse(m_Father->GetRotation()) * new_rotation;
		}
		MakeDirty();
	}


	// The rotation of the transform relative to the parent transform's rotation.
	Quaternion GetLocalRotation() const
	{
		return m_LocalRotation;
	}

	void SetLocalRotation(const Quaternion& rotation)
	{
		m_LocalRotation = rotation;
		MakeDirty();
	}

	// The scale of the transform relative to the parent.
	Vector3 GetLocalScale() const
	{
		return m_LocalScale;
	}

	void SetLocalScale(const Vector3& scale)
	{
		m_LocalScale = scale;
		MakeDirty();
	}

	void SetLocalScale(const float x, const float y, const float z)
	{
		m_LocalScale.Set(x, y, z);
		MakeDirty();
	}

	void SetLocalScale(const float scale)
	{
		m_LocalScale.x = m_LocalScale.y = m_LocalScale.z = scale;
		MakeDirty();
	}

	// The global scale of the object(Read Only).
	Vector3 GetLossyScale()
	{
		auto p = GetParent();
		if (p != nullptr)
			return m_LocalScale * p->GetLossyScale();
		return m_LocalScale;
	}

	// direction (1, 0, 0) in world space.
	Vector3 GetRight() const
	{
		return GetRotation() * Vector3::right;
	}

	// direction (0, 1, 0) in world space
	Vector3 GetUp() const
	{
		return GetRotation() * Vector3::up;
	}

	// direction (0, 0, 1) in world space.
	Vector3 GetForward() const
	{
		return GetRotation() * Vector3::forward;
	}

	void SetForward(Vector3 const & newForward)
	{
		//this->LookAt(position() + newForward, this->up());
		SetRotation(Quaternion::LookRotation(newForward));
	}

	Transform* GetParent() const
	{
		return m_Father;
	}

	// parent: The parent Transform to use.
	// worldPositionStays: If true, the parent-relative position, scale and rotation are modified such that the object keeps the same world space position, rotation and scale as before.
	void SetParent(Transform* parent, bool worldPositionStays = true);


	Matrix4x4 GetWorldToLocalMatrix() const
	{
		UpdateMatrix();
//		return m_worldToLocalMatrix;
		return m_LocalToWorldMatrix.inverse();
	}

	void SetWorldToLocalMatrix(const Matrix4x4& worldToLocal)
	{
		SetLocalToWorldMatrix(worldToLocal.inverse());
	}


	// Matrix that transforms a point from local space into world space (Read Only).
	const Matrix4x4& GetLocalToWorldMatrix() const
	{
		UpdateMatrix();
		return m_LocalToWorldMatrix;
	}

	void SetLocalToWorldMatrix(const Matrix4x4& localToWorld)
	{
		auto mat = localToWorld;
		if (m_Father != nullptr)
			mat = m_Father->GetWorldToLocalMatrix() * mat;	// local to parent
		Matrix4x4::Decompose(mat, &m_LocalPosition, &m_LocalRotation, &m_LocalScale);
		MakeDirty();
	}


	// Applies a rotation of zAngle degrees around the z axis, xAngle degrees around the x axis, and yAngle degrees around the y axis (in that order)
	//		void Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo = Space::Self);

	// Rotates the transform about axis passing through point in world coordinates by angle degrees.
	void RotateAround(const Vector3& point, const Vector3& axis, float angle);


	void UpdateMatrix() const;

	int GetRootOrder() const { return m_RootOrder; }
	void SetRootOrder(int index);

	const std::vector<Transform*>& GetChildren() const
	{
		return m_Children;
	}

	Transform* GetChildAt(int index) const
	{
		return m_Children[index];
	}

	void SetSiblingIndex(int index);
	int GetSiblingIndex() const;
	
	//const Matrix4x4& GetLocalToWorldMatrix() const { return m_LocalToWorldMatrix; }
	//
	//Quaternion GetRotation() const { return m_LocalToWorldMatrix.ToRotation(); }
	//Vector3 GetUp() const { return GetRotation() * Vector3::up; }
	//Vector3 GetRight() const { return GetRotation() * Vector3::right; }
	//Vector3 GetForward() const { return GetRotation() * Vector3::forward; }
	//
	//// TODO: move to TransformSystem
	Vector3 TransformDirection(const Vector3& direction) const;
	void Translate(const Vector3& translation, Space relativeTo = Space::Self);
	//void RotateAround(const Vector3& point, const Vector3& axis, float angle);
	
protected:

	Quaternion m_LocalRotation{ 0, 0, 0, 1 };
	Vector3 m_LocalPosition{ 0, 0, 0 };
	Vector3 m_LocalScale{ 1, 1, 1 };

	std::vector<Transform*> m_Children;
	Transform* m_Father = nullptr;
	int m_RootOrder = 0;

	mutable bool m_IsDirty = true;
	mutable Matrix4x4 m_LocalToWorldMatrix;
//	mutable Matrix4x4 m_worldToLocalMatrix;

	void MakeDirty() const;
};
