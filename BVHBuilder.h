#pragma once

#include <vector>

#include "Face.h"
#include "BVHNode.h"

class BVHBuilder
{
public:

	BVHBuilder(const std::vector<Face>& triangles);

	const TriangleNode& GetTriangleNode(uint32_t index) const;
	const InnerNode& GetInnerNode(uint32_t index) const;

private:

	ConstructResult ConstructNode(uint32_t firstTriIndex, uint32_t triCount);
	AABB CalculateAABB(uint32_t firstTriIndex);

	std::vector<TriangleNode> m_triNodes;
	std::vector<InnerNode> m_innerNodes;
	std::vector<Face> m_triangles;
};

