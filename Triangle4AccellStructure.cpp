#include "Triangle4AccellStructure.h"

#include "Material4Index.h"
#include "TraversalTriangle.h"
#include "TraversalTriangle4.h"
#include "TriangleTexCoords.h"
#include "TriangleTexCoords4.h"

// --------------------------------------------------------------------------------
Triangle4AccellStructure::Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles, 
	std::vector<uint32_t> perTriangleMaterials, std::vector<TriangleTexCoords> triangleTexCoords)
{
	// Pad to a multiple of 4, if needed
	const uint32_t remainder = (uint32_t)traversalTriangles.size() % 4u;
	if (remainder != 0u)
	{
		const TraversalTriangle traversalTriangle;
		const TriangleTexCoords triTexCoords;

		const uint32_t padCount = 4u - remainder;
		for (uint32_t padding = 0u; padding < padCount; padding++)
		{
			traversalTriangles.push_back(traversalTriangle);
			perTriangleMaterials.push_back(UINT32_MAX);
			triangleTexCoords.push_back(triTexCoords);

		}
	}

	// Traversal data
	TraversalTriangle4 traversalTriangle4;
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

	// Material indices
	Material4Index material4Index;
	for (uint32_t index = 0u; index < perTriangleMaterials.size(); index += 4u)
	{
		material4Index.m_indices[0u] = perTriangleMaterials[index];
		material4Index.m_indices[1u] = perTriangleMaterials[index + 1u];
		material4Index.m_indices[2u] = perTriangleMaterials[index + 2u];
		material4Index.m_indices[3u] = perTriangleMaterials[index + 3u];

		m_perTriangle4Materials.push_back(material4Index);
	}

	// Texture coordinates
	TriangleTexCoords4 triangleTexCoords4;
	for (uint32_t index = 0u; index < triangleTexCoords.size(); index += 4u)
	{
		triangleTexCoords4.m_v0U[0u] = triangleTexCoords[index].m_v0uv[0u];
		triangleTexCoords4.m_v0V[0u] = triangleTexCoords[index].m_v0uv[1u];

		triangleTexCoords4.m_v0U[1u] = triangleTexCoords[index + 1u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[1u] = triangleTexCoords[index + 1u].m_v0uv[1u];

		triangleTexCoords4.m_v0U[2u] = triangleTexCoords[index + 2u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[2u] = triangleTexCoords[index + 2u].m_v0uv[1u];

		triangleTexCoords4.m_v0U[3u] = triangleTexCoords[index + 2u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[3u] = triangleTexCoords[index + 2u].m_v0uv[1u];
	}
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& Triangle4AccellStructure::GetTraversalTriangle4s() const
{
	return m_traversalTriangle4s;
}

// --------------------------------------------------------------------------------
const std::vector<Material4Index>& Triangle4AccellStructure::GetMaterial4Indices() const
{
	return m_perTriangle4Materials;
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoords4>& Triangle4AccellStructure::GetTriangleTexCoords4() const
{
	return m_triangleTexCoords4;
}
