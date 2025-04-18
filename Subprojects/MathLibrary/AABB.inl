#include "AABB.h"
// --------------------------------------------------------------------------------
inline AABB::AABB() : m_min(INFINITY, INFINITY, INFINITY),
m_max(-INFINITY, -INFINITY, -INFINITY)
{

}

// --------------------------------------------------------------------------------
inline AABB::AABB(const Vector3& min, const Vector3& max)
{
	m_min = min;
	m_max = max;
}

// --------------------------------------------------------------------------------
inline void AABB::MergeAABB(struct AABB aabb)
{
	m_min = Min(m_min, aabb.m_min);
	m_max = Max(m_max, aabb.m_max);
}

// --------------------------------------------------------------------------------
inline float AABB::GetSurfaceArea() const
{
	const float width = m_max.X() - m_min.X();
	const float height = m_max.Y() - m_min.Y();
	const float length = m_max.Z() - m_min.Z();

	return 2.0f * ((width * length) + (height * length) + (height * width));
}