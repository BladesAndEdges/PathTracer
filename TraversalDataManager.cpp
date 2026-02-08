#include "TraversalDataManager.h"

#include "BVH2AccellStructure.h"
#include "BVH4AccellStructure.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"
#include "TriangleAccellStructure.h"
#include "Triangle4AccellStructure.h"

// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> perTriangleMaterials)
{
	m_triangleAccellStructure = new TriangleAccellStructure(triangles, perTriangleMaterials);
	m_triangle4AccellStructure = new Triangle4AccellStructure(m_triangleAccellStructure->GetTraversalTriangles(), m_triangleAccellStructure->GetPerTriangleMaterials(), 
		m_triangleAccellStructure->GetTriangleTexCoords());
	m_bvh2AccellStructure = new BVH2AccellStructure(triangles, m_triangleAccellStructure->GetTraversalTriangles(), m_triangleAccellStructure->GetPerTriangleMaterials(),
		m_triangleAccellStructure->GetTriangleTexCoords(), BVH2PartitionStrategy::HalfWayLongestAxisWithSAH);
	m_bvh4AccellStructure = new BVH4AccellStructure(m_bvh2AccellStructure);
}

// --------------------------------------------------------------------------------
const uint32_t TraversalDataManager::GetTraversalTrianglesCount() const
{
	return (uint32_t)m_triangleAccellStructure->GetTraversalTrianglesCount();
}

// --------------------------------------------------------------------------------
const TraversalTriangle& TraversalDataManager::GetTraversalTriangle(const uint32_t index) const
{
	return m_triangleAccellStructure->GetTraversalTriangle(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoords& TraversalDataManager::GetTriangleTexCoords(const uint32_t index) const
{
	return m_triangleAccellStructure->GetTriangleTexCoords(index);
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TraversalDataManager::GetTraversalTriangles() const
{
	return m_triangleAccellStructure->GetTraversalTriangles();
}

// --------------------------------------------------------------------------------
const std::vector<uint32_t>& TraversalDataManager::GetMaterialIndices() const
{
	return m_triangleAccellStructure->GetPerTriangleMaterials();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoords>& TraversalDataManager::GetTriangleTexCoords() const
{
	return m_triangleAccellStructure->GetTriangleTexCoords();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& TraversalDataManager::GetTraversalTriangle4s() const
{
	return m_triangle4AccellStructure->GetTraversalTriangle4s();
}

// --------------------------------------------------------------------------------
const std::vector<Material4Index>& TraversalDataManager::GetMaterial4Indices() const
{
	return m_triangle4AccellStructure->GetMaterial4Indices();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoords4>& TraversalDataManager::GetTriangleTexCoords4() const
{
	return m_triangle4AccellStructure->GetTriangleTexCoords4();
}

// --------------------------------------------------------------------------------
const BVH2InnerNode& TraversalDataManager::GetBVH2InnerNode(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetInnerNode(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle& TraversalDataManager::GetBVH2TraversalTriangle(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetTraversalTriangle(index);
}

// --------------------------------------------------------------------------------
const uint32_t TraversalDataManager::GetBVH2MaterialIndex(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetMaterialIndex(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoords& TraversalDataManager::GetBVH2TriangleTexCoords(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetTriangleTexCoords(index);
}

// --------------------------------------------------------------------------------
const BVH4InnerNode& TraversalDataManager::GetBVH4InnerNode(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetInnerNodeTri4(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle4& TraversalDataManager::GetBVH4TraversalTriangle4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTraversalTriangle4(index);
}

// --------------------------------------------------------------------------------
const TriangleIndices& TraversalDataManager::GetBVH4TriangleIndices(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTriangleIndices(index);
}

// --------------------------------------------------------------------------------
const Material4Index& TraversalDataManager::GetBVH4Material4Index(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetMaterial4Index(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoords4& TraversalDataManager::GetBVH4TriangleTexCoords4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTriangleTexCoords4(index);
}
