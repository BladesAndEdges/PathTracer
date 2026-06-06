#include "TraversalDataManager.h"

#include "BVH2AccelStructure.h"
#include "BVH4AccelStructure.h"
#include "ScalarAccelStructure.h"
#include "SSEAccelStructure.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"


// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> materials)
{
	m_scalarAccelStructure = new ScalarAccelStructure(triangles, materials);
	m_sseAccelStructure = new SSEAccelStructure(m_scalarAccelStructure->GetTraversalTriangles(), m_scalarAccelStructure->GetMaterialIndices(), 
		m_scalarAccelStructure->GetTriangleTexCoords());
	m_bvh2AccelStructure = new BVH2AccelStructure(triangles, m_scalarAccelStructure->GetTraversalTriangles(), m_scalarAccelStructure->GetMaterialIndices(),
		m_scalarAccelStructure->GetTriangleTexCoords(), BVH2PartitionStrategy::HalfWayLongestAxisWithSAH);
	m_bvh4AccelStructure = new BVH4AccelStructure(m_bvh2AccelStructure);
}

// --------------------------------------------------------------------------------
const uint32_t TraversalDataManager::GetTraversalTrianglesCount() const
{
	return (uint32_t)m_scalarAccelStructure->GetTraversalTrianglesCount();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TraversalDataManager::GetTraversalTriangles() const
{
	return m_scalarAccelStructure->GetTraversalTriangles();
}

// --------------------------------------------------------------------------------
const std::vector<uint32_t>& TraversalDataManager::GetMaterialIndices() const
{
	return m_scalarAccelStructure->GetMaterialIndices();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoord>& TraversalDataManager::GetTriangleTexCoords() const
{
	return m_scalarAccelStructure->GetTriangleTexCoords();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& TraversalDataManager::GetSSETraversalTriangle4s() const
{
	return m_sseAccelStructure->GetTraversalTriangle4s();
}

// --------------------------------------------------------------------------------
const std::vector<MaterialIndex4>& TraversalDataManager::GetSSEMaterialIndex4s() const
{
	return m_sseAccelStructure->GetMaterialIndex4s();
}

// --------------------------------------------------------------------------------
const std::vector<TriangleTexCoord4>& TraversalDataManager::GetSSETriangleTexCoord4s() const
{
	return m_sseAccelStructure->GetTriangleTexCoord4s();
}

// --------------------------------------------------------------------------------
const BVH2Node& TraversalDataManager::GetBVH2Node(const uint32_t index) const
{
	return m_bvh2AccelStructure->GetBVH2Node(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle& TraversalDataManager::GetBVH2TraversalTriangle(const uint32_t index) const
{
	return m_bvh2AccelStructure->GetTraversalTriangle(index);
}

// --------------------------------------------------------------------------------
const uint32_t TraversalDataManager::GetBVH2MaterialIndex(const uint32_t index) const
{
	return m_bvh2AccelStructure->GetMaterialIndex(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoord& TraversalDataManager::GetBVH2TriangleTexCoord(const uint32_t index) const
{
	return m_bvh2AccelStructure->GetTriangleTexCoord(index);
}

// --------------------------------------------------------------------------------
const BVH4Node& TraversalDataManager::GetBVH4Node(const uint32_t index) const
{
	return m_bvh4AccelStructure->GetBVH4Node(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle4& TraversalDataManager::GetBVH4TraversalTriangle4(const uint32_t index) const
{
	return m_bvh4AccelStructure->GetTraversalTriangle4(index);
}

// --------------------------------------------------------------------------------
const TriangleIndex4& TraversalDataManager::GetBVH4TriangleIndex4(const uint32_t index) const
{
	return m_bvh4AccelStructure->GetTriangleIndex4(index);
}

// --------------------------------------------------------------------------------
const MaterialIndex4& TraversalDataManager::GetBVH4MaterialIndex4(const uint32_t index) const
{
	return m_bvh4AccelStructure->GetMaterialIndex4(index);
}

// --------------------------------------------------------------------------------
const TriangleTexCoord4& TraversalDataManager::GetBVH4TriangleTexCoord4(const uint32_t index) const
{
	return m_bvh4AccelStructure->GetTriangleTexCoord4(index);
}
