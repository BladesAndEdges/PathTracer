#include "TraversalDataManager.h"

#include "BVH2AccellStructure.h"
#include "BVH4AccellStructure.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"
#include "TriangleAccellStructure.h"
#include "Triangle4AccellStructure.h"

// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> materials)
{
	m_triangleAccellStructure = new TriangleAccellStructure(triangles, materials);
	m_triangle4AccellStructure = new Triangle4AccellStructure(m_triangleAccellStructure->GetTraversalTriangles(), m_triangleAccellStructure->GetMaterialIndices(), 
		m_triangleAccellStructure->GetTriangleTexCoords());
	m_bvh2AccellStructure = new BVH2AccellStructure(triangles, m_triangleAccellStructure->GetTraversalTriangles(), m_triangleAccellStructure->GetMaterialIndices(),
		m_triangleAccellStructure->GetTriangleTexCoords(), BVH2PartitionStrategy::HalfWayLongestAxisWithSAH);
	m_bvh4AccellStructure = new BVH4AccellStructure(m_bvh2AccellStructure);
}

// --------------------------------------------------------------------------------
const uint32_t TraversalDataManager::GetTraversalTrianglesCount() const
{
	return (uint32_t)m_triangleAccellStructure->GetTraversalTrianglesCount();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TraversalDataManager::GetTraversalTriangles() const
{
	return m_triangleAccellStructure->GetTraversalTriangles();
}

// --------------------------------------------------------------------------------
const std::vector<uint32_t>& TraversalDataManager::GetMaterialIndices() const
{
	return m_triangleAccellStructure->GetMaterialIndices();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoord>& TraversalDataManager::GetTriangleTexCoords() const
{
	return m_triangleAccellStructure->GetTriangleTexCoords();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& TraversalDataManager::GetSSETraversalTriangle4s() const
{
	return m_triangle4AccellStructure->GetTraversalTriangle4s();
}

// --------------------------------------------------------------------------------
const std::vector<MaterialIndex4>& TraversalDataManager::GetSSEMaterialIndex4s() const
{
	return m_triangle4AccellStructure->GetMaterialIndex4s();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoord4>& TraversalDataManager::GetSSETriangleTexCoord4s() const
{
	return m_triangle4AccellStructure->GetTriangleTexCoord4s();
}

// --------------------------------------------------------------------------------
const BVH2Node& TraversalDataManager::GetBVH2Node(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetBVH2Node(index);
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
const TriangleTexCoord& TraversalDataManager::GetBVH2TriangleTexCoord(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetTriangleTexCoord(index);
}

// --------------------------------------------------------------------------------
const BVH4Node& TraversalDataManager::GetBVH4Node(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetBVH4Node(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle4& TraversalDataManager::GetBVH4TraversalTriangle4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTraversalTriangle4(index);
}

// --------------------------------------------------------------------------------
const TriangleIndex4& TraversalDataManager::GetBVH4TriangleIndex4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTriangleIndex4(index);
}

// --------------------------------------------------------------------------------
const MaterialIndex4& TraversalDataManager::GetBVH4MaterialIndex4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetMaterialIndex4(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoord4& TraversalDataManager::GetBVH4TriangleTexCoord4(const uint32_t index) const
{
	return m_bvh4AccellStructure->GetTriangleTexCoord4(index);
}
