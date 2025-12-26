#include "Triangle4AccellStructure.h"

#include "TraversalTriangle.h"
#include "Triangle4.h"

// --------------------------------------------------------------------------------
Triangle4AccellStructure::Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles)
{
	// Pad to a multiple of 4, if needed
	const uint32_t remainder = (uint32_t)traversalTriangles.size() % 4u;
	if (remainder != 0u)
	{
		const uint32_t padCount = 4u - remainder;
		const TraversalTriangle traversalTriangle;
		for (uint32_t padding = 0u; padding < padCount; padding++)
		{
			traversalTriangles.push_back(traversalTriangle);
		}
	}

	Triangle4 traversalTriangle4;
	for (uint32_t index = 0u; index < traversalTriangles.size(); index += 4u)
	{
		// First triangle
		traversalTriangle4.m_v0X[0u] = traversalTriangles[index].m_v0[0u];
		traversalTriangle4.m_v0Y[0u] = traversalTriangles[index].m_v0[1u];
		traversalTriangle4.m_v0Z[0u] = traversalTriangles[index].m_v0[2u];

		traversalTriangle4.m_edge1X[0u] = traversalTriangles[index].m_edge1[0u];
		traversalTriangle4.m_edge1Y[0u] = traversalTriangles[index].m_edge1[1u];
		traversalTriangle4.m_edge1Z[0u] = traversalTriangles[index].m_edge1[2u];

		traversalTriangle4.m_edge2X[0u] = traversalTriangles[index].m_edge2[0u];
		traversalTriangle4.m_edge2Y[0u] = traversalTriangles[index].m_edge2[1u];
		traversalTriangle4.m_edge2Z[0u] = traversalTriangles[index].m_edge2[2u];

		// Second triangle
		traversalTriangle4.m_v0X[1u] = traversalTriangles[index + 1u].m_v0[0u];
		traversalTriangle4.m_v0Y[1u] = traversalTriangles[index + 1u].m_v0[1u];
		traversalTriangle4.m_v0Z[1u] = traversalTriangles[index + 1u].m_v0[2u];

		traversalTriangle4.m_edge1X[1u] = traversalTriangles[index + 1u].m_edge1[0u];
		traversalTriangle4.m_edge1Y[1u] = traversalTriangles[index + 1u].m_edge1[1u];
		traversalTriangle4.m_edge1Z[1u] = traversalTriangles[index + 1u].m_edge1[2u];

		traversalTriangle4.m_edge2X[1u] = traversalTriangles[index + 1u].m_edge2[0u];
		traversalTriangle4.m_edge2Y[1u] = traversalTriangles[index + 1u].m_edge2[1u];
		traversalTriangle4.m_edge2Z[1u] = traversalTriangles[index + 1u].m_edge2[2u];

		// Third triangle
		traversalTriangle4.m_v0X[2u] = traversalTriangles[index + 2u].m_v0[0u];
		traversalTriangle4.m_v0Y[2u] = traversalTriangles[index + 2u].m_v0[1u];
		traversalTriangle4.m_v0Z[2u] = traversalTriangles[index + 2u].m_v0[2u];

		traversalTriangle4.m_edge1X[2u] = traversalTriangles[index + 2u].m_edge1[0u];
		traversalTriangle4.m_edge1Y[2u] = traversalTriangles[index + 2u].m_edge1[1u];
		traversalTriangle4.m_edge1Z[2u] = traversalTriangles[index + 2u].m_edge1[2u];

		traversalTriangle4.m_edge2X[2u] = traversalTriangles[index + 2u].m_edge2[0u];
		traversalTriangle4.m_edge2Y[2u] = traversalTriangles[index + 2u].m_edge2[1u];
		traversalTriangle4.m_edge2Z[2u] = traversalTriangles[index + 2u].m_edge2[2u];

		// Fourth triangle
		traversalTriangle4.m_v0X[3u] = traversalTriangles[index + 3u].m_v0[0u];
		traversalTriangle4.m_v0Y[3u] = traversalTriangles[index + 3u].m_v0[1u];
		traversalTriangle4.m_v0Z[3u] = traversalTriangles[index + 3u].m_v0[2u];

		traversalTriangle4.m_edge1X[3u] = traversalTriangles[index + 3u].m_edge1[0u];
		traversalTriangle4.m_edge1Y[3u] = traversalTriangles[index + 3u].m_edge1[1u];
		traversalTriangle4.m_edge1Z[3u] = traversalTriangles[index + 3u].m_edge1[2u];

		traversalTriangle4.m_edge2X[3u] = traversalTriangles[index + 3u].m_edge2[0u];
		traversalTriangle4.m_edge2Y[3u] = traversalTriangles[index + 3u].m_edge2[1u];
		traversalTriangle4.m_edge2Z[3u] = traversalTriangles[index + 3u].m_edge2[2u];

		m_traversalTriangle4s.push_back(traversalTriangle4);
	}
}

// --------------------------------------------------------------------------------
const std::vector<Triangle4>& Triangle4AccellStructure::GetTraversalTriangle4s() const
{
	return m_traversalTriangle4s;
}
