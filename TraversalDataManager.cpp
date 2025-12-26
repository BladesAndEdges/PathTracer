#include "TraversalDataManager.h"

#include "TraversalTriangle.h"
#include "TriangleAccellStructure.h"
#include "Triangle4AccellStructure.h"

// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles)
{
	m_triangleAccellStructure = new TriangleAccellStructure(triangles);
	m_triangle4AccellStructure = new Triangle4AccellStructure(m_triangleAccellStructure->GetTraversalTriangles());
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
