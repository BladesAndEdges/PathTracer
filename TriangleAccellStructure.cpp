#include "TriangleAccellStructure.h"

#include "TraversalTriangle.h"
#include "Triangle.h"
#include "TriangleTexCoords.h"

// --------------------------------------------------------------------------------
TriangleAccellStructure::TriangleAccellStructure(const std::vector<Triangle>& triangles, const std::vector<uint32_t>& perTriangleMaterials) : m_perTriangleMaterials(perTriangleMaterials)
{
	// Traversal data
	TraversalTriangle traversalTriangle;
	for (uint32_t triangle = 0u; triangle < triangles.size(); triangle++)
	{
		traversalTriangle.m_v0[0u] = triangles[triangle].m_vertices[0u].m_position[0u];
		traversalTriangle.m_v0[1u] = triangles[triangle].m_vertices[0u].m_position[1u];
		traversalTriangle.m_v0[2u] = triangles[triangle].m_vertices[0u].m_position[2u];

		traversalTriangle.m_edge1[0u] = triangles[triangle].m_vertices[1u].m_position[0u] - triangles[triangle].m_vertices[0u].m_position[0u];
		traversalTriangle.m_edge1[1u] = triangles[triangle].m_vertices[1u].m_position[1u] - triangles[triangle].m_vertices[0u].m_position[1u];
		traversalTriangle.m_edge1[2u] = triangles[triangle].m_vertices[1u].m_position[2u] - triangles[triangle].m_vertices[0u].m_position[2u];

		traversalTriangle.m_edge2[0u] = triangles[triangle].m_vertices[2u].m_position[0u] - triangles[triangle].m_vertices[0u].m_position[0u];
		traversalTriangle.m_edge2[1u] = triangles[triangle].m_vertices[2u].m_position[1u] - triangles[triangle].m_vertices[0u].m_position[1u];
		traversalTriangle.m_edge2[2u] = triangles[triangle].m_vertices[2u].m_position[2u] - triangles[triangle].m_vertices[0u].m_position[2u];

		m_traversalTriangles.push_back(traversalTriangle);
	}

	// Texture coordinate data
	TriangleTexCoords triangleTexCoords;
	for (uint32_t triangle = 0u; triangle < triangles.size(); triangle++)
	{
		triangleTexCoords.m_v0uv[0u] = triangles[triangle].m_vertices[0u].m_textureCoordinate[0u];
		triangleTexCoords.m_v0uv[1u] = triangles[triangle].m_vertices[0u].m_textureCoordinate[1u];

		triangleTexCoords.m_v1uv[0u] = triangles[triangle].m_vertices[1u].m_textureCoordinate[0u];
		triangleTexCoords.m_v1uv[1u] = triangles[triangle].m_vertices[1u].m_textureCoordinate[1u];

		triangleTexCoords.m_v2uv[0u] = triangles[triangle].m_vertices[2u].m_textureCoordinate[0u];
		triangleTexCoords.m_v2uv[1u] = triangles[triangle].m_vertices[2u].m_textureCoordinate[1u];

		m_triangleTexCoords.push_back(triangleTexCoords);
	}
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TriangleAccellStructure::GetTraversalTriangles() const
{
	return m_traversalTriangles;
}

// --------------------------------------------------------------------------------
const std::vector<uint32_t>& TriangleAccellStructure::GetPerTriangleMaterials() const
{
	return m_perTriangleMaterials;
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoords>& TriangleAccellStructure::GetTriangleTexCoords() const
{
	return m_triangleTexCoords;
}
