#pragma once

#include <FishEngine/Components/Animator.hpp>
#include <FishEngine/ECS/System.hpp>
#include <FishEngine/Components/Transform.hpp>

namespace FishEngine
{

inline bool IsValid(const Vector3& v)
{
	return !v.hasNaNs() && !(std::isinf(v.x) && std::isinf(v.y) && std::isinf(v.z));
}

inline bool IsValid(const Quaternion& v)
{
	return !v.hasNaNs() && !(std::isinf(v.x) && std::isinf(v.y) && std::isinf(v.z));
}

class AnimationSystem : public System
{
	SYSTEM(AnimationSystem);
public:
	void OnAdded() override
	{
	}

	void Update() override
	{
		//float time = glfwGetTime();
		m_Scene->ForEach<Animator>([](GameObject* go, Animator* animator)
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

}
