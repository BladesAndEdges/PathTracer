#pragma once

#include<vector>

struct TraversalTriangle;
struct Triangle;

// --------------------------------------------------------------------------------
class TriangleAccellStructure
{
public:

	TriangleAccellStructure(const std::vector<Triangle>& triangles);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;

private:

	std::vector<TraversalTriangle> m_traversalTriangles;
};

