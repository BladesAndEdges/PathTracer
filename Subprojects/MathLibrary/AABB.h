#pragma once

#include "Vector3.h"

// --------------------------------------------------------------------------------
struct AABB
{

public:

	inline AABB();
	inline AABB(const Vector3& min, const Vector3& max);

	Vector3 m_min;
	Vector3 m_max;

	// Perhaps inline?
	inline void MergeAABB(struct AABB aabb);
	inline float GetSurfaceArea() const;
};

#include "AABB.inl"