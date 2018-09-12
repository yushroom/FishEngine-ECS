#pragma once

#include "Vector3.hpp"
//#include "Macro.hpp"

namespace FishEngine
{
	class Ray
	{
	public:
		//InjectSerializationFunctionsNonVirtual(Ray)
		
		// The origin point of the ray.
		Vector3 origin {0, 0, 0};

		// The direction of the ray.
		Vector3 direction {0, 0, 1};
		
		Ray() = default;

		// Creates a ray starting at origin along direction.
		Ray(const Vector3& origin, const Vector3& direction)
			: origin(origin), direction(direction.normalized())
		{

		}

		// Returns a point at distance units along the ray.
		Vector3 GetPoint(float distance)
		{
			return origin + direction * distance;
		}

		bool IntersectSphere(const Vector3& center, const float radius, float* t) const;
		
		bool IntersectPlane(const Vector3& normal, const Vector3& point, float* t) const;
	};
}
