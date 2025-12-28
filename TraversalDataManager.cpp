#include "TraversalDataManager.h"

#include "BVH2AccellStructure.h"
#include "BVH4AccellStructure.h"
#include "TraversalTriangle.h"
#include "TriangleAccellStructure.h"
#include "Triangle4AccellStructure.h"

// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles)
{
	m_triangleAccellStructure = new TriangleAccellStructure(triangles);
	m_triangle4AccellStructure = new Triangle4AccellStructure(m_triangleAccellStructure->GetTraversalTriangles());
	m_bvh2AccellStructure = new BVH2AccellStructure(triangles, m_triangleAccellStructure->GetTraversalTriangles(), BVH2PartitionStrategy::HalfWayLongestAxisWithSAH);
	m_bvh4AccellStructure = new BVH4AccellStructure(m_bvh2AccellStructure);
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TraversalDataManager::GetTraversalTriangles() const
{
	return m_triangleAccellStructure->GetTraversalTriangles();
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle4>& TraversalDataManager::GetTraversalTriangle4s() const
{
	return m_triangle4AccellStructure->GetTraversalTriangle4s();
}

// --------------------------------------------------------------------------------
const BVH2InnerNode& TraversalDataManager::GetBVH2InnerNode(const uint32_t index) const
{
	return m_bvh2AccellStructure->GetInnerNode(index);
}

// --------------------------------------------------------------------------------
const TraversalTriangle& TraversalDataManager::GetBVH2TraversalTriangle(const uint32_t index) const
{
	// TO DO: assert the index is valid
	return m_bvh2AccellStructure->GetTraversalTriangle(index);
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
