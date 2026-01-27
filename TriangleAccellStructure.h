#pragma once

#include<vector>

struct TraversalTriangle;
struct Triangle;

// --------------------------------------------------------------------------------
class TriangleAccellStructure
{
public:

	TriangleAccellStructure(const std::vector<Triangle>& triangles, const std::vector<uint32_t>& perTriangleMaterials);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<uint32_t>& GetPerTriangleMaterials() const;

private:

	std::vector<TraversalTriangle> m_traversalTriangles;
	std::vector<uint32_t> m_perTriangleMaterials;
};

