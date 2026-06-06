#pragma once

#include <vector>

class BVH2AccellStructure;
class BVH4AccellStructure;
struct BVH2Node;
struct BVH4Node;
struct MaterialIndex4;
class ScalarAccelStructure;
struct TraversalTriangle;
struct TraversalTriangle4;
struct TriangleIndex4;
struct Triangle;
struct TriangleTexCoord;
struct TriangleTexCoord4;
class Triangle4AccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> materials);

	const uint32_t GetTraversalTrianglesCount() const;
	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<uint32_t>& GetMaterialIndices() const;
	const std::vector<TriangleTexCoord>& GetTriangleTexCoords() const;

	const std::vector<TraversalTriangle4>& GetSSETraversalTriangle4s() const;
	const std::vector<MaterialIndex4>& GetSSEMaterialIndex4s() const;
	const std::vector<TriangleTexCoord4>& GetSSETriangleTexCoord4s() const;

	const BVH2Node& GetBVH2Node(const uint32_t index) const;
	const TraversalTriangle& GetBVH2TraversalTriangle(const uint32_t index) const;
	const uint32_t GetBVH2MaterialIndex(const uint32_t index) const;
	const TriangleTexCoord& GetBVH2TriangleTexCoord(const uint32_t index) const;

	const BVH4Node& GetBVH4Node(const uint32_t index) const;
	const TraversalTriangle4& GetBVH4TraversalTriangle4(const uint32_t index) const;
	const TriangleIndex4& GetBVH4TriangleIndex4(const uint32_t index) const;
	const MaterialIndex4& GetBVH4MaterialIndex4(const uint32_t index) const;
	const TriangleTexCoord4& GetBVH4TriangleTexCoord4(const uint32_t index) const;

private:

	ScalarAccelStructure* m_scalarAccelStructure;
	Triangle4AccellStructure* m_triangle4AccellStructure;
	BVH2AccellStructure* m_bvh2AccellStructure;
	BVH4AccellStructure* m_bvh4AccellStructure;
};

