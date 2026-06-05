#pragma once

#include <vector>

struct MaterialIndex4;
struct TraversalTriangle;
struct TraversalTriangle4;
struct TriangleTexCoords;
struct TriangleTexCoord4;

// --------------------------------------------------------------------------------
class Triangle4AccellStructure
{
public:

	Triangle4AccellStructure(std::vector<TraversalTriangle> traversalTriangles, std::vector<uint32_t> perTriangleMaterials, 
		std::vector<TriangleTexCoords> triangleTexCoords);

	const std::vector<MaterialIndex4>& GetMaterialIndex4s() const;
	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const std::vector<TriangleTexCoord4>& GetTriangleTexCoord4s() const;

private:

	std::vector<MaterialIndex4> m_materialIndex4s;
	std::vector<TraversalTriangle4> m_traversalTriangle4s;
	std::vector<TriangleTexCoord4> m_triangleTexCoord4s;
};

