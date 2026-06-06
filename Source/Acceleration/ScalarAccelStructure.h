#pragma once

#include<vector>

struct TraversalTriangle;
struct Triangle;
struct TriangleTexCoord;

// --------------------------------------------------------------------------------
class ScalarAccelStructure
{
public:

	ScalarAccelStructure(const std::vector<Triangle>& triangles, const std::vector<uint32_t>& perTriangleMaterials);

	const uint32_t GetTraversalTrianglesCount() const;
	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<uint32_t>& GetMaterialIndices() const;
	const std::vector<TriangleTexCoord>& GetTriangleTexCoords() const;

private:

	std::vector<TraversalTriangle> m_traversalTriangles;
	std::vector<uint32_t> m_materialIndices;
	std::vector<TriangleTexCoord> m_triangleTexCoords;
};

