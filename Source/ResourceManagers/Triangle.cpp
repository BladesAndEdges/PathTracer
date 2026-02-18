#include "Triangle.h"

#include "AABB.h"

// --------------------------------------------------------------------------------
AABB CalculateTriangleAABB(const Triangle& triangle)
{
	AABB aabb;

	aabb.m_min.SetX(triangle.m_vertices[0u].m_position[0u]);
	aabb.m_min.SetY(triangle.m_vertices[0u].m_position[1u]);
	aabb.m_min.SetZ(triangle.m_vertices[0u].m_position[2u]);

	aabb.m_max.SetX(triangle.m_vertices[0u].m_position[0u]);
	aabb.m_max.SetY(triangle.m_vertices[0u].m_position[1u]);
	aabb.m_max.SetZ(triangle.m_vertices[0u].m_position[2u]);

	for (uint32_t vertex = 1u; vertex < 3u; vertex++)
	{
		const Vector3 currentVertex(triangle.m_vertices[vertex].m_position[0u],
			triangle.m_vertices[vertex].m_position[1u],
			triangle.m_vertices[vertex].m_position[2u]);

		aabb.m_min = Min(aabb.m_min, currentVertex);
		aabb.m_max = Max(aabb.m_max, currentVertex);
	}

	return aabb;
}
