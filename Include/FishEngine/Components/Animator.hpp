#pragma once

#include "../Object.hpp"
#include "../ECS/Component.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector3.hpp"
#include <cassert>

namespace FishEngine
{
	enum class AnimationCurveType
	{
		Translation,
		Rotation,
		Scale,
		Weights
	};

	
	struct AnimationCurve
	{
		AnimationCurveType type;
		GameObject* node = nullptr;
		std::vector<float> input;
		std::vector<float> output;

		Quaternion SampleQuat(float time, const Quaternion& initValue);

		Vector3 SampleVector3(float time, const Vector3& initValue);

	private:
		Vector3 _GetVector3(size_t idx);

		Quaternion _GetQuaternion(size_t idx);
	};

	
	class AnimationClip : public Object
	{
		friend class ModelUtil;
	public:
		std::vector<AnimationCurve> curves;
		float length = 0;
	};

	
	class Animator : public Component
	{
		COMPONENT(Animator);
	public:
		AnimationClip * GetCurrentClip() const
		{
			if (m_Clips.empty() || m_CurrentClipIndex == -1)
				return nullptr;
			assert(m_CurrentClipIndex >= 0 && m_CurrentClipIndex < m_Clips.size());
			return m_Clips[m_CurrentClipIndex];
		}

		std::vector<AnimationClip*> m_Clips;
		int m_CurrentClipIndex = -1;
		float m_LocalTimer = 0;
	};
}
