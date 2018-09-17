#include <FishEngine/Components/Transform.hpp>
#include <cassert>

using namespace FishEngine;

Vector3 Transform::TransformPoint(const Vector3& point) const
{
	return GetLocalToWorldMatrix().MultiplyPoint(point);
}

Vector3 Transform::TransformDirection(const Vector3& direction) const
{
	return GetLocalToWorldMatrix().MultiplyVector(direction);
}

void Transform::Translate(const Vector3& translation, Space relativeTo /*= Space::Self*/)
{
	if (relativeTo == Space::World)
		SetPosition(GetPosition() + translation);
	else
		//SetPosition(GetPosition() + TransformDirection(translation));
		SetLocalPosition(m_LocalPosition + translation);
}


void Transform::LookAt(const Vector3& worldPosition, const Vector3& worldUp /*= Vector3::up*/)
{
	auto rot = Matrix4x4::LookAt(GetPosition(), worldPosition, worldUp).inverse().ToRotation();
	SetRotation(rot);
	//SetForward((worldPosition - GetPosition()).normalized());
}

//
//void Transform::RotateAround(const Vector3& point, const Vector3& axis, float angle)
//{
//	// step1: update position
//	auto vector = this->position;
//	auto rot = Quaternion::AngleAxis(angle, axis);
//	Vector3 vector2 = vector - point;
//	vector2 = rot * vector2;
//	vector = point + vector2;
//	position = vector;
//
//	// step2: update rotation
//	rotation = rot * rotation;
//}

//namespace FishEngine
//{
	//Transform::Transform() : Component(Transform::ClassID, ClassName)
	//{
	//	LOGF;
	//	//		auto scene = SceneManager::GetActiveScene();
	//	//		scene->AddRootTransform(this);
	//}

	//Transform::~Transform()
	//{
	//	LOGF;
	//	//		for (auto c : m_children)
	//	for (int i = (int)m_Children.size() - 1; i >= 0; --i)
	//	{
	//		delete m_Children[i]->GetGameObject();
	//	}
	//	//		m_children.clear();
	//	// TODO
	//	SetParent(nullptr); // remove from parent
	//}

	//void Transform::SetRootOrder(int index)
	//{
	//	assert(index >= 0);
	//	if (m_Father != nullptr)
	//	{
	//		// rootOrder in .unity file may > total size
	//		//				if (index >= m_parent->m_children.size())
	//		//					index = m_parent->m_children.size()-1;
	//		assert(index < m_Father->m_Children.size());
	//	}
	//	if (index == m_RootOrder)
	//		return;
	//	int old = m_RootOrder;
	//	auto& c = m_Father == nullptr ?
	//		m_GameObject->GetScene()->m_RootTransforms :
	//		m_Father->m_Children;
	//	c[index]->m_RootOrder = old;
	//	std::swap(c[index], c[old]);
	//	m_RootOrder = index;
	//}


	void Transform::RotateAround(const Vector3& point, const Vector3& axis, float angle)
	{
		// step1: update position
		auto vector = this->GetPosition();
		auto rotation = Quaternion::AngleAxis(angle, axis);
		Vector3 vector2 = vector - point;
		vector2 = rotation * vector2;
		vector = point + vector2;
		SetPosition(vector);

		// step2: update rotation
		m_LocalRotation = rotation * m_LocalRotation;
		//RotateAroundInternal(axis, angle);
		MakeDirty();
	}


	void Transform::UpdateMatrix() const
	{
		if (!m_IsDirty)
			return;
#if 1
		m_LocalToWorldMatrix.SetTRS(m_LocalPosition, m_LocalRotation, m_LocalScale);
		if (m_Father != nullptr) {
			m_LocalToWorldMatrix = m_Father->GetLocalToWorldMatrix() * m_LocalToWorldMatrix;
		}
//		m_worldToLocalMatrix = m_localToWorldMatrix.inverse();
#else
		// TODO this version is not right, take a look to see where the bug is.
		// maybe in the TRS
		Matrix4x4::TRS(m_LocalPosition, m_LocalRotation, m_LocalScale, m_LocalToWorldMatrix, m_worldToLocalMatrix);
		if (!m_Father.expired()) {
			m_LocalToWorldMatrix = m_Father.lock()->localToWorldMatrix() * m_LocalToWorldMatrix;
			m_worldToLocalMatrix = m_worldToLocalMatrix * m_Father.lock()->worldToLocalMatrix();
		}
#endif
		m_IsDirty = false;
	}


	void Transform::SetParent(Transform* parent, bool worldPositionStays)
	{
		auto old_parent = m_Father;
		if (parent == old_parent)
		{
			return;
		}

		if (parent == nullptr)
		{
			auto scene = m_GameObject->GetScene();
			scene->AddRootTransform(this);
		}
		else if (old_parent == nullptr)
		{
			m_GameObject->GetScene()->RemoveRootTransform(this);
		}

		// new parent can not be child of this
		auto p = parent;
		while (p != nullptr)
		{
			if (p == this)
			{
				puts("new parent can not be child of this");
				return;
			}
			p = p->GetParent();
		}


		// remove from old parent
		if (old_parent != nullptr)
		{
			//p->m_children.remove(this);
			// TODO: remove first, not remove all
			//			old_parent->m_children.erase(old_parent->m_children.begin()+m_RootOrder);
			std::remove(old_parent->m_Children.begin(), old_parent->m_Children.end(), this);
		}

		// old_parent.localToWorld * old_localToWorld = new_parent.localToWorld * new_localToWorld
		// ==> new_localToWorld = new_parent.worldToLocal * old_parent.localToWorld * old_localToWorld

		m_Father = parent;
		if (parent != nullptr)
		{
			parent->m_Children.push_back(this);
			m_RootOrder = (int)parent->m_Children.size() - 1;
		}

		if (worldPositionStays)
		{
			Matrix4x4 mat = Matrix4x4::TRS(m_LocalPosition, m_LocalRotation, m_LocalScale);
			if (old_parent != nullptr)
				mat = old_parent->GetLocalToWorldMatrix() * mat;
			if (parent != nullptr)
				mat = parent->GetWorldToLocalMatrix() * mat;
			Matrix4x4::Decompose(mat, &m_LocalPosition, &m_LocalRotation, &m_LocalScale);
		}
		//UpdateMatrix();
		MakeDirty();
	}


	void Transform::MakeDirty() const
	{
		if (!m_IsDirty)
		{
			for (auto& c : m_Children)
			{
				c->MakeDirty();
			}
			m_IsDirty = true;
		}
	}


	void Transform::SetSiblingIndex(int index)
	{
		auto parent = GetParent();
		auto & children = parent != nullptr ?
			parent->m_Children :
			m_GameObject->GetScene()->m_RootTransforms;

		assert(index >= 0 && index < children.size());

		auto pos = std::find(children.begin(), children.end(), this);
		int old_index = (int)std::distance(children.begin(), pos);
		//		int old_index = GetSiblingIndex();
		if (old_index < index)
		{
			// [old_index ... index] -> [old_index+1 ... index, old_index]
			//			auto b = children.begin();
			//			std::rotate(b+old_index, b+(old_index+1), b+index);

			for (int i = index - 1; i >= old_index; --i)
			{
				children[i] = children[i + 1];
			}
			children[index] = this;
		}
		else if (old_index > index)
		{
			// [index ... old_index] -> [old_index, index ... old_index-1]
			for (int i = old_index; i > index; --i)
			{
				children[i] = children[i - 1];
			}
			children[index] = this;
		}
		m_RootOrder = index;
	}


	int Transform::GetSiblingIndex() const
	{
		auto parent = GetParent();
		auto & children = parent != nullptr ?
			parent->GetChildren() :
			m_GameObject->GetScene()->GetRootTransforms();
		//		std::sort(children.begin(), children.end(), [](Transform* a, Transform* b) {
		//			return a->m_RootOrder < b->m_RootOrder;
		//		});
		//
		//		for (int i = 0; i < children.size(); ++i)
		//		{
		//			auto t = children[i];
		//			t->m_RootOrder = i;
		//		}
		//		return m_RootOrder;
		auto pos = std::find(children.begin(), children.end(), this);
		return (int)std::distance(children.begin(), pos);
	}
//}
