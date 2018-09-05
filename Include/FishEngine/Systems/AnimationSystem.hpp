#pragma once

#include <GLFW/glfw3.h>
#include <FishEngine/Components/Animation.hpp>

class AnimationSystem : public ECS::ISystem
{
public:
	void OnAdded() override
	{
	}

	void Update() override
	{
		float time = glfwGetTime();
		m_Scene->ForEach<Animation>([time](ECS::GameObject* go, Animation* animation)
		{
			float tt = time;
			while (tt > animation->length)
				tt -= animation->length;
			for (auto& curve : animation->curves)
			{
				auto t = curve.node->GetTransform();
				if (curve.type == AnimationCurveType::Translation)
				{
					Vector3 p = curve.SampleVector3(tt, Vector3::zero);
					t->SetLocalPosition(p);
				}
				else if (curve.type == AnimationCurveType::Rotation)
				{
					Quaternion q = curve.SampleQuat(tt, Quaternion::identity);
					t->SetLocalRotation(q);
				}
				else if (curve.type == AnimationCurveType::Scale)
				{
					Vector3 s = curve.SampleVector3(tt, Vector3::one);
					t->SetLocalScale(s);
				}
			}
		});
	}
};
