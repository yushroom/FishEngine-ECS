#pragma once

#include <FishEngine/Components/Animator.hpp>

inline bool IsValid(const Vector3& v)
{
	return !v.hasNaNs() && !(isinf(v.x) && isinf(v.y) && isinf(v.z));
}

inline bool IsValid(const Quaternion& v)
{
	return !v.hasNaNs() && !(isinf(v.x) && isinf(v.y) && isinf(v.z));
}

class AnimationSystem : public ECS::ISystem
{
	SYSTEM(AnimationSystem);
public:
	void OnAdded() override
	{
	}

	void Update() override
	{
		//float time = glfwGetTime();
		m_Scene->ForEach<Animator>([](ECS::GameObject* go, Animator* animator)
		{
			auto& tt = animator->m_LocalTimer;
			tt += 0.01667f;
			auto clip = animator->GetCurrentClip();
			if (tt > clip->length)
			{
				tt = 0;
				animator->m_CurrentClipIndex++;
				animator->m_CurrentClipIndex %= animator->m_Clips.size();
				clip = animator->GetCurrentClip();
			}
			for (auto& curve : clip->curves)
			{
				auto t = curve.node->GetTransform();
				if (curve.type == AnimationCurveType::Translation)
				{
					Vector3 p = curve.SampleVector3(tt, t->GetLocalPosition());
					assert(IsValid(p));
					t->SetLocalPosition(p);
				}
				else if (curve.type == AnimationCurveType::Rotation)
				{
					Quaternion q = curve.SampleQuat(tt, t->GetLocalRotation());
					assert(IsValid(q));
					t->SetLocalRotation(q);
				}
				else if (curve.type == AnimationCurveType::Scale)
				{
					Vector3 s = curve.SampleVector3(tt, t->GetLocalScale());
					assert(IsValid(s));
					t->SetLocalScale(s);
				}
			}
		});
	}
};
