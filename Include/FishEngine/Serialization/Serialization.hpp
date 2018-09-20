#include "../ECS/Component.hpp"

namespace FishEngine
{
	class Vector2;
	FishEngine::InputArchive&  operator>>(FishEngine::InputArchive& archive, Vector2& t);
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector2& t);

	class Vector3;
	FishEngine::InputArchive&  operator>>(FishEngine::InputArchive& archive, Vector3& t);
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector3& t);

	class Vector4;
	FishEngine::InputArchive&  operator>>(FishEngine::InputArchive& archive, Vector4& t);
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector4& t);

	class Quaternion;
	FishEngine::InputArchive&  operator>>(FishEngine::InputArchive& archive, Quaternion& t);
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Quaternion& t);
}