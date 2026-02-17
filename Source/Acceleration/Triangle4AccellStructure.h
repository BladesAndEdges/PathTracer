#pragma once

#include <vector>

struct Material4Index;
struct TraversalTriangle;
struct TraversalTriangle4;
struct TriangleTexCoords;
struct TriangleTexCoords4;

// --------------------------------------------------------------------------------
class Triangle4AccellStructure
{
public:

	Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles, std::vector<uint32_t> perTriangleMaterials, 
		std::vector<TriangleTexCoords> triangleTexCoords);

	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const std::vector<Material4Index>& GetMaterial4Indices() const;
	const std::vector<TriangleTexCoords4>& GetTriangleTexCoords4() const;

private:

	std::vector<TraversalTriangle4> m_traversalTriangle4s;
	std::vector<Material4Index> m_perTriangle4Materials;
	std::vector<TriangleTexCoords4> m_triangleTexCoords4;
};

