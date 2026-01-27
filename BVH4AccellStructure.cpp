#include "BVH4AccellStructure.h"

#include "AABB.h"
#include "BVH2AccellStructure.h"
#include "Material4Index.h"
#include "TraversalTriangle.h"
#include "TraversalTriangle4.h"

// --------------------------------------------------------------------------------
BVH4AccellStructure::BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure)
{
	assert(bvh2AccellStructure != nullptr);
	// Add getter for the size of the internal node vector, and check it is not empty
	const uint32_t bvhRootIndex = BuildBVH4NodeFromBVH2NodeTri4(bvh2AccellStructure, 0);

	assert(m_traversalTriangle4s.size() == m_triangleIndices.size());
	assert(m_traversalTriangle4s.size() == m_material4Indices.size());

	(void)bvhRootIndex;
}

// --------------------------------------------------------------------------------
void RecursiveGetChildren(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t& root, const AABB& box, const uint32_t depth, uint32_t* children, 
	AABB* boxes, uint32_t& addedChildren, uint32_t& triangleMask)
{
	assert(bvh2AccellStructure != nullptr);
	assert(depth <= 2u);
	assert(children != nullptr);
	assert(addedChildren < 4u);
	assert(triangleMask <= 15u);

	const bool isTriangle = root >> 31u;

	if (isTriangle || (depth > 1u))
	{
		children[addedChildren] = root;
		boxes[addedChildren] = box;

		if (isTriangle)
		{
			triangleMask = triangleMask | (1u << (3u - addedChildren));
			assert(triangleMask <= 15u);
		}

		addedChildren++;
	}
	else
	{
		const BVH2InnerNode rootNode = bvh2AccellStructure->GetInnerNode(root);

		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_leftChild, rootNode.m_leftAABB, depth + 1, children, boxes, addedChildren, triangleMask);
		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_rightChild, rootNode.m_rightAABB, depth + 1, children, boxes, addedChildren, triangleMask);
	}
}

// --------------------------------------------------------------------------------
uint32_t BVH4AccellStructure::BuildBVH4NodeFromBVH2NodeTri4(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t bvh2SubtreeRootIndex)
{
	assert((bvh2SubtreeRootIndex >> 31u) != 1u);

	uint32_t children[4u];
	AABB boxes[4u];
	uint32_t addedChildren = 0u;
	uint32_t triangleMask = 0u;
	RecursiveGetChildren(bvh2AccellStructure, bvh2SubtreeRootIndex, AABB(), 0u, children, boxes, addedChildren, triangleMask);

	const uint32_t bvh4Node = (uint32_t)m_innerNodesTri4.size();
	m_innerNodesTri4.push_back(BVH4InnerNode());
	uint32_t subNode = 0u;

	// Handle the triangles
	if (triangleMask)
	{
		TraversalTriangle4 triangles;
		AABB trianglesAABB;
		uint32_t subTriangle = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				const uint32_t indexInBVH2 = children[child] & ~(1u << 31u);
				const TraversalTriangle& traversalTriangle = bvh2AccellStructure->GetTraversalTriangle(indexInBVH2);

				triangles.m_v0X[subTriangle] = traversalTriangle.m_v0[0u];
				triangles.m_v0Y[subTriangle] = traversalTriangle.m_v0[1u];
				triangles.m_v0Z[subTriangle] = traversalTriangle.m_v0[2u];

				triangles.m_edge1X[subTriangle] = traversalTriangle.m_edge1[0u];
				triangles.m_edge1Y[subTriangle] = traversalTriangle.m_edge1[1u];
				triangles.m_edge1Z[subTriangle] = traversalTriangle.m_edge1[2u];

				triangles.m_edge2X[subTriangle] = traversalTriangle.m_edge2[0u];
				triangles.m_edge2Y[subTriangle] = traversalTriangle.m_edge2[1u];
				triangles.m_edge2Z[subTriangle] = traversalTriangle.m_edge2[2u];

				trianglesAABB.MergeAABB(boxes[child]);

				subTriangle++;
			}
		}

		// Add dummy triangles if neccessary
		for (uint32_t triangle = subTriangle; triangle < 4u; triangle++)
		{
			triangles.m_v0X[triangle] = std::nanf("");
			triangles.m_v0Y[triangle] = std::nanf("");
			triangles.m_v0Z[triangle] = std::nanf("");

			triangles.m_edge1X[triangle] = std::nanf("");
			triangles.m_edge1Y[triangle] = std::nanf("");
			triangles.m_edge1Z[triangle] = std::nanf("");

			triangles.m_edge2X[triangle] = std::nanf("");
			triangles.m_edge2Y[triangle] = std::nanf("");
			triangles.m_edge2Z[triangle] = std::nanf("");
		}

		// Add the triangle indices associated with the triangle4
		TriangleIndices triangleIndices;
		uint32_t triangleIndex = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				triangleIndices.m_triangleIndices[triangleIndex] = children[child] & ~(1u << 31u);
				triangleIndex++;
			}
		}

		// Add the materials associated with the triangle4
		Material4Index material4Index;
		uint32_t material = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				const uint32_t indexInBVH2 = children[child] & ~(1u << 31u);
				material4Index.m_indices[material] = bvh2AccellStructure->GetMaterialIndex(indexInBVH2);
				material++;
			}
		}

		const uint32_t triangle4Index = (uint32_t)m_traversalTriangle4s.size();
		m_traversalTriangle4s.push_back(triangles);
		m_triangleIndices.push_back(triangleIndices);
		m_material4Indices.push_back(material4Index);

		// Place the triangle4 as a node child
		m_innerNodesTri4[bvh4Node].m_child[subNode] = triangle4Index | (1u << 31u);

		m_innerNodesTri4[bvh4Node].m_aabbMinX[subNode] = trianglesAABB.m_min.X();
		m_innerNodesTri4[bvh4Node].m_aabbMinY[subNode] = trianglesAABB.m_min.Y();
		m_innerNodesTri4[bvh4Node].m_aabbMinZ[subNode] = trianglesAABB.m_min.Z();
		m_innerNodesTri4[bvh4Node].m_aabbMaxX[subNode] = trianglesAABB.m_max.X();
		m_innerNodesTri4[bvh4Node].m_aabbMaxY[subNode] = trianglesAABB.m_max.Y();
		m_innerNodesTri4[bvh4Node].m_aabbMaxZ[subNode] = trianglesAABB.m_max.Z();

		subNode++;
	}

	// Handle the regular nodes
	for (uint32_t child = 0u; child < addedChildren; child++)
	{
		const uint32_t postShiftValue = triangleMask >> (3u - child);
		if (!(postShiftValue & 1u))
		{
			m_innerNodesTri4[bvh4Node].m_child[subNode] = BuildBVH4NodeFromBVH2NodeTri4(bvh2AccellStructure, children[child]);
			
			m_innerNodesTri4[bvh4Node].m_aabbMinX[subNode] = boxes[child].m_min.X();
			m_innerNodesTri4[bvh4Node].m_aabbMinY[subNode] = boxes[child].m_min.Y();
			m_innerNodesTri4[bvh4Node].m_aabbMinZ[subNode] = boxes[child].m_min.Z();

			m_innerNodesTri4[bvh4Node].m_aabbMaxX[subNode] = boxes[child].m_max.X();
			m_innerNodesTri4[bvh4Node].m_aabbMaxY[subNode] = boxes[child].m_max.Y();
			m_innerNodesTri4[bvh4Node].m_aabbMaxZ[subNode] = boxes[child].m_max.Z();

			subNode++;
		}
	}

	//Populate the remaining space with dummy nodes
	for (; subNode < 4u; subNode++)
	{
		m_innerNodesTri4[bvh4Node].m_child[subNode] = 0x7fffffffu;

		m_innerNodesTri4[bvh4Node].m_aabbMinX[subNode] = std::nanf("");
		m_innerNodesTri4[bvh4Node].m_aabbMinY[subNode] = std::nanf("");
		m_innerNodesTri4[bvh4Node].m_aabbMinZ[subNode] = std::nanf("");

		m_innerNodesTri4[bvh4Node].m_aabbMaxX[subNode] = std::nanf("");
		m_innerNodesTri4[bvh4Node].m_aabbMaxY[subNode] = std::nanf("");
		m_innerNodesTri4[bvh4Node].m_aabbMaxZ[subNode] = std::nanf("");
	}

	return bvh4Node;
}

// --------------------------------------------------------------------------------
const BVH4InnerNode& BVH4AccellStructure::GetInnerNodeTri4(const uint32_t index) const
{
	assert(index < m_innerNodesTri4.size());
	return m_innerNodesTri4[index];
}

// --------------------------------------------------------------------------------
const TraversalTriangle4& BVH4AccellStructure::GetTraversalTriangle4(const uint32_t index) const
{
	assert(index < (uint32_t)m_traversalTriangle4s.size());
	return m_traversalTriangle4s[index];
}

// --------------------------------------------------------------------------------
const TriangleIndices& BVH4AccellStructure::GetTriangleIndices(const uint32_t index) const
{
	assert(index < (uint32_t)m_triangleIndices.size());
	return m_triangleIndices[index];
}

// --------------------------------------------------------------------------------
const Material4Index& BVH4AccellStructure::GetMaterial4Index(const uint32_t index) const
{
	assert(index < (uint32_t)m_material4Indices.size());
	return m_material4Indices[index];
}
