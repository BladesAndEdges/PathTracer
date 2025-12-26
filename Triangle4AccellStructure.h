#pragma once

#include <vector>

struct TraversalTriangle;
struct Triangle4;

// --------------------------------------------------------------------------------
class Triangle4AccellStructure
{
public:

	Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles);

	const std::vector<Triangle4>& GetTraversalTriangle4s() const;

private:

	std::vector<Triangle4> m_traversalTriangle4s;
};

