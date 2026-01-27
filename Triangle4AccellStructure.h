#pragma once

#include <vector>

struct Material4Index;
struct TraversalTriangle;
struct TraversalTriangle4;

// --------------------------------------------------------------------------------
class Triangle4AccellStructure
{
public:

	Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles, std::vector<uint32_t> perTriangleMaterials);

	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const std::vector<Material4Index>& GetMaterial4Indices() const;

private:

	std::vector<TraversalTriangle4> m_traversalTriangle4s;
	std::vector<Material4Index> m_perTriangle4Materials;
};

