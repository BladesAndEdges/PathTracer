#include "Triangle4AccellStructure.h"

#include "BaseTypes4.h"
#include "TraversalTriangle.h"
#include "TraversalTriangle4.h"
#include "TriangleTexCoords.h"

// --------------------------------------------------------------------------------
Triangle4AccellStructure::Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles, 
	std::vector<uint32_t> triangleMaterials, std::vector<TriangleTexCoords> triangleTexCoords)
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
			triangleMaterials.push_back(UINT32_MAX);
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
	MaterialIndex4 materialIndex4;
	for (uint32_t index = 0u; index < triangleMaterials.size(); index += 4u)
	{
		materialIndex4.m_index[0u] = triangleMaterials[index];
		materialIndex4.m_index[1u] = triangleMaterials[index + 1u];
		materialIndex4.m_index[2u] = triangleMaterials[index + 2u];
		materialIndex4.m_index[3u] = triangleMaterials[index + 3u];

		m_materialIndex4s.push_back(materialIndex4);
	}

	// Texture coordinates
	TriangleTexCoord4 triangleTexCoords4;
	for (uint32_t index = 0u; index < triangleTexCoords.size(); index += 4u)
	{
		// Tex coord 0
		triangleTexCoords4.m_v0U[0u] = triangleTexCoords[index].m_v0uv[0u];
		triangleTexCoords4.m_v0V[0u] = triangleTexCoords[index].m_v0uv[1u];

		triangleTexCoords4.m_v1U[0u] = triangleTexCoords[index].m_v1uv[0u];
		triangleTexCoords4.m_v1V[0u] = triangleTexCoords[index].m_v1uv[1u];

		triangleTexCoords4.m_v2U[0u] = triangleTexCoords[index].m_v2uv[0u];
		triangleTexCoords4.m_v2V[0u] = triangleTexCoords[index].m_v2uv[1u];

		// Tex coord 1
		triangleTexCoords4.m_v0U[1u] = triangleTexCoords[index + 1u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[1u] = triangleTexCoords[index + 1u].m_v0uv[1u];

		triangleTexCoords4.m_v1U[1u] = triangleTexCoords[index + 1u].m_v1uv[0u];
		triangleTexCoords4.m_v1V[1u] = triangleTexCoords[index + 1u].m_v1uv[1u];

		triangleTexCoords4.m_v2U[1u] = triangleTexCoords[index + 1u].m_v2uv[0u];
		triangleTexCoords4.m_v2V[1u] = triangleTexCoords[index + 1u].m_v2uv[1u];

		// Tex coord 2
		triangleTexCoords4.m_v0U[2u] = triangleTexCoords[index + 2u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[2u] = triangleTexCoords[index + 2u].m_v0uv[1u];

		triangleTexCoords4.m_v1U[2u] = triangleTexCoords[index + 2u].m_v1uv[0u];
		triangleTexCoords4.m_v1V[2u] = triangleTexCoords[index + 2u].m_v1uv[1u];

		triangleTexCoords4.m_v2U[2u] = triangleTexCoords[index + 2u].m_v2uv[0u];
		triangleTexCoords4.m_v2V[2u] = triangleTexCoords[index + 2u].m_v2uv[1u];

		// Tex coord 3
		triangleTexCoords4.m_v0U[3u] = triangleTexCoords[index + 3u].m_v0uv[0u];
		triangleTexCoords4.m_v0V[3u] = triangleTexCoords[index + 3u].m_v0uv[1u];

		triangleTexCoords4.m_v1U[3u] = triangleTexCoords[index + 3u].m_v1uv[0u];
		triangleTexCoords4.m_v1V[3u] = triangleTexCoords[index + 3u].m_v1uv[1u];

		triangleTexCoords4.m_v2U[3u] = triangleTexCoords[index + 3u].m_v2uv[0u];
		triangleTexCoords4.m_v2V[3u] = triangleTexCoords[index + 3u].m_v2uv[1u];

		m_triangleTexCoord4s.push_back(triangleTexCoords4);
	}
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& Triangle4AccellStructure::GetTraversalTriangle4s() const
{
	return m_traversalTriangle4s;
}

// --------------------------------------------------------------------------------
const std::vector<MaterialIndex4>& Triangle4AccellStructure::GetMaterialIndex4s() const
{
	return m_materialIndex4s;
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoord4>& Triangle4AccellStructure::GetTriangleTexCoord4s() const
{
	return m_triangleTexCoord4s;
}
