#pragma once

#include <vector>

struct TraversalTriangle;
struct Triangle;
class TriangleAccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;

private:

	TriangleAccellStructure* m_triangleAccellStructure;
};

