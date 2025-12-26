#pragma once

#include <vector>

struct TraversalTriangle;
struct TraversalTriangle4;
struct Triangle;
class TriangleAccellStructure;
class Triangle4AccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;

private:

	TriangleAccellStructure* m_triangleAccellStructure;
	Triangle4AccellStructure* m_triangle4AccellStructure;
};

