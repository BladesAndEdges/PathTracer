#pragma once

#include <vector>

struct TraversalTriangle;
struct TraversalTriangle4;

// --------------------------------------------------------------------------------
class Triangle4AccellStructure
{
public:

	Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles);

	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;

private:

	std::vector<TraversalTriangle4> m_traversalTriangle4s;
};

