#include "TraversalDataManager.h"

#include "TriangleAccellStructure.h"

// --------------------------------------------------------------------------------
TraversalDataManager::TraversalDataManager(const std::vector<Triangle>& triangles)
{
	m_triangleAccellStructure = new TriangleAccellStructure(triangles);
}

// --------------------------------------------------------------------------------
const std::vector<TraversalTriangle>& TraversalDataManager::GetTraversalTriangles() const
{
	return m_triangleAccellStructure->GetTraversalTriangles();
}
