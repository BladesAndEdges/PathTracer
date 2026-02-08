#pragma once

#include<vector>

struct TraversalTriangle;
struct Triangle;
struct TriangleTexCoords;

// --------------------------------------------------------------------------------
class TriangleAccellStructure
{
public:

	TriangleAccellStructure(const std::vector<Triangle>& triangles, const std::vector<uint32_t>& perTriangleMaterials);

	const uint32_t GetTraversalTrianglesCount() const;
	const TraversalTriangle& GetTraversalTriangle(const uint32_t index) const;
	const TriangleTexCoords& GetTriangleTexCoords(const uint32_t index) const;
	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<uint32_t>& GetPerTriangleMaterials() const;
	const std::vector<TriangleTexCoords>& GetTriangleTexCoords() const;

private:

	std::vector<TraversalTriangle> m_traversalTriangles;
	std::vector<uint32_t> m_perTriangleMaterials;
	std::vector<TriangleTexCoords> m_triangleTexCoords;
};

