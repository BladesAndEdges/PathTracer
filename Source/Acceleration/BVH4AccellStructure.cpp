#include "BVH4AccellStructure.h"

#include <assert.h>

#include "BaseTypes4.h"
#include "BVH2AccelStructure.h"
#include "BVH2Node.h"
#include "BVH4Node.h"
#include "TraversalTriangle.h"
#include "TraversalTriangle4.h"
#include "TriangleTexCoords.h"

// --------------------------------------------------------------------------------
BVH4AccellStructure::BVH4AccellStructure(const BVH2AccelStructure* bvh2AccelStructure)
{
	assert(bvh2AccelStructure != nullptr);

	const uint32_t bvhRootIndex = MakeBVH4Node(bvh2AccelStructure, 0u);
	(void)bvhRootIndex;

	assert(m_traversalTriangle4s.size() == m_triangleIndex4s.size());
	assert(m_traversalTriangle4s.size() == m_materialIndex4s.size());
}

// --------------------------------------------------------------------------------
void GetChildren(const BVH2AccelStructure* bvh2AccelStructure, uint32_t* children,
	AABB* boxes, uint32_t& addedChildren, uint32_t& triangleMask)
{
	while (addedChildren < 4u)
	{
		float largestSurfaceArea = 0.0f;
		uint32_t childIndex = UINT32_MAX;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			if ((children[child] >> 31u) != 1u)
			{
				const BVH2Node node = bvh2AccelStructure->GetBVH2Node(children[child]);

				AABB childrenAABB;
				childrenAABB.MergeAABB(node.m_leftAABB);
				childrenAABB.MergeAABB(node.m_rightAABB);

				if (childrenAABB.GetSurfaceArea() > largestSurfaceArea)
				{
					childIndex = child;
					largestSurfaceArea = childrenAABB.GetSurfaceArea();
				}
			}
		}

		// If no node available, break
		if (childIndex == UINT32_MAX)
		{
			break;
		}
		else
		{
			const BVH2Node node = bvh2AccelStructure->GetBVH2Node(children[childIndex]);

			children[childIndex] = node.m_leftChild;
			boxes[childIndex] = node.m_leftAABB;

			children[addedChildren] = node.m_rightChild;
			boxes[addedChildren] = node.m_rightAABB;

			addedChildren++;
		}
	}

	// Update the triangle mask if any children are triangles
	for (uint32_t child = 0u; child < addedChildren; child++)
	{
		if ((children[child] >> 31u) == 1u)
		{
			triangleMask = triangleMask = triangleMask | (1u << (3u - child));
		}
	}
}

// --------------------------------------------------------------------------------
uint32_t BVH4AccellStructure::MakeBVH4Node(const BVH2AccelStructure* bvh2AccelStructure, const uint32_t bvh2SubtreeRootIndex)
{
	assert((bvh2SubtreeRootIndex >> 31u) != 1u);

	uint32_t children[4u];
	children[0u] = bvh2SubtreeRootIndex;
	
	AABB boxes[4u];
	uint32_t addedChildren = 1u;
	uint32_t triangleMask = 0u;
	
	GetChildren(bvh2AccelStructure, children, boxes, addedChildren, triangleMask);
	
	const uint32_t bvh4Node = (uint32_t)m_bvh4Nodes.size();
	m_bvh4Nodes.push_back(BVH4Node());
	uint32_t subNode = 0u;

	// Handle the triangles
	if (triangleMask)
	{
		TraversalTriangle4 traversalTriangle4;
		AABB trianglesAABB;
		uint32_t subTriangle = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				const uint32_t indexInBVH2 = children[child] & ~(1u << 31u);
				const TraversalTriangle& traversalTriangle = bvh2AccelStructure->GetTraversalTriangle(indexInBVH2);

				traversalTriangle4.m_v0X[subTriangle] = traversalTriangle.m_v0[0u];
				traversalTriangle4.m_v0Y[subTriangle] = traversalTriangle.m_v0[1u];
				traversalTriangle4.m_v0Z[subTriangle] = traversalTriangle.m_v0[2u];

				traversalTriangle4.m_edge1X[subTriangle] = traversalTriangle.m_edge1[0u];
				traversalTriangle4.m_edge1Y[subTriangle] = traversalTriangle.m_edge1[1u];
				traversalTriangle4.m_edge1Z[subTriangle] = traversalTriangle.m_edge1[2u];

				traversalTriangle4.m_edge2X[subTriangle] = traversalTriangle.m_edge2[0u];
				traversalTriangle4.m_edge2Y[subTriangle] = traversalTriangle.m_edge2[1u];
				traversalTriangle4.m_edge2Z[subTriangle] = traversalTriangle.m_edge2[2u];

				trianglesAABB.MergeAABB(boxes[child]);

				subTriangle++;
			}
		}

		// Add dummy triangles if neccessary
		for (uint32_t triangle = subTriangle; triangle < 4u; triangle++)
		{
			traversalTriangle4.m_v0X[triangle] = std::nanf("");
			traversalTriangle4.m_v0Y[triangle] = std::nanf("");
			traversalTriangle4.m_v0Z[triangle] = std::nanf("");

			traversalTriangle4.m_edge1X[triangle] = std::nanf("");
			traversalTriangle4.m_edge1Y[triangle] = std::nanf("");
			traversalTriangle4.m_edge1Z[triangle] = std::nanf("");

			traversalTriangle4.m_edge2X[triangle] = std::nanf("");
			traversalTriangle4.m_edge2Y[triangle] = std::nanf("");
			traversalTriangle4.m_edge2Z[triangle] = std::nanf("");
		}

		// Add the triangle indices associated with the triangle4
		TriangleIndex4 triangleIndex4;
		uint32_t triangleIndex = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				triangleIndex4.m_index[triangleIndex] = children[child] & ~(1u << 31u);
				triangleIndex++;
			}
		}

		// Add the materials associated with the triangle4
		MaterialIndex4 materialIndex4;
		uint32_t material = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				const uint32_t indexInBVH2 = children[child] & ~(1u << 31u);
				materialIndex4.m_index[material] = bvh2AccelStructure->GetMaterialIndex(indexInBVH2);
				material++;
			}
		}

		// Triangle tex coords
		TriangleTexCoord4 triangleTexCoord4;
		uint32_t texCoord = 0u;
		for (uint32_t child = 0u; child < addedChildren; child++)
		{
			const uint32_t postShiftValue = triangleMask >> (3u - child);
			if (postShiftValue & 1u)
			{
				const uint32_t indexInBVH2 = children[child] & ~(1u << 31u);
				const TriangleTexCoord& triangleTexCoord = bvh2AccelStructure->GetTriangleTexCoord(indexInBVH2);

				triangleTexCoord4.m_v0U[texCoord] = triangleTexCoord.m_v0uv[0u];
				triangleTexCoord4.m_v0V[texCoord] = triangleTexCoord.m_v0uv[1u];

				triangleTexCoord4.m_v1U[texCoord] = triangleTexCoord.m_v1uv[0u];
				triangleTexCoord4.m_v1V[texCoord] = triangleTexCoord.m_v1uv[1u];

				triangleTexCoord4.m_v2U[texCoord] = triangleTexCoord.m_v2uv[0u];
				triangleTexCoord4.m_v2V[texCoord] = triangleTexCoord.m_v2uv[1u];

				texCoord++;
			}
		}

		const uint32_t triangle4Index = (uint32_t)m_traversalTriangle4s.size();
		m_traversalTriangle4s.push_back(traversalTriangle4);
		m_triangleIndex4s.push_back(triangleIndex4);
		m_materialIndex4s.push_back(materialIndex4);
		m_triangleTexCoord4s.push_back(triangleTexCoord4);

		// Place the triangle4 as a node child
		m_bvh4Nodes[bvh4Node].m_child[subNode] = triangle4Index | (1u << 31u);

		m_bvh4Nodes[bvh4Node].m_aabbMinX[subNode] = trianglesAABB.m_min.X();
		m_bvh4Nodes[bvh4Node].m_aabbMinY[subNode] = trianglesAABB.m_min.Y();
		m_bvh4Nodes[bvh4Node].m_aabbMinZ[subNode] = trianglesAABB.m_min.Z();
		m_bvh4Nodes[bvh4Node].m_aabbMaxX[subNode] = trianglesAABB.m_max.X();
		m_bvh4Nodes[bvh4Node].m_aabbMaxY[subNode] = trianglesAABB.m_max.Y();
		m_bvh4Nodes[bvh4Node].m_aabbMaxZ[subNode] = trianglesAABB.m_max.Z();

		m_bvh4Nodes[bvh4Node].m_validity[subNode] = 0xffffffff;

		subNode++;
	}

	// Handle the regular nodes
	for (uint32_t child = 0u; child < addedChildren; child++)
	{
		const uint32_t postShiftValue = triangleMask >> (3u - child);
		if (!(postShiftValue & 1u))
		{
			m_bvh4Nodes[bvh4Node].m_child[subNode] = MakeBVH4Node(bvh2AccelStructure, children[child]);
			
			m_bvh4Nodes[bvh4Node].m_aabbMinX[subNode] = boxes[child].m_min.X();
			m_bvh4Nodes[bvh4Node].m_aabbMinY[subNode] = boxes[child].m_min.Y();
			m_bvh4Nodes[bvh4Node].m_aabbMinZ[subNode] = boxes[child].m_min.Z();

			m_bvh4Nodes[bvh4Node].m_aabbMaxX[subNode] = boxes[child].m_max.X();
			m_bvh4Nodes[bvh4Node].m_aabbMaxY[subNode] = boxes[child].m_max.Y();
			m_bvh4Nodes[bvh4Node].m_aabbMaxZ[subNode] = boxes[child].m_max.Z();

			m_bvh4Nodes[bvh4Node].m_validity[subNode] = 0xffffffff;

			subNode++;
		}
	}

	//Populate the remaining space with dummy nodes
	for (; subNode < 4u; subNode++)
	{
		m_bvh4Nodes[bvh4Node].m_child[subNode] = 0x7fffffffu;

		m_bvh4Nodes[bvh4Node].m_aabbMinX[subNode] = std::nanf("");
		m_bvh4Nodes[bvh4Node].m_aabbMinY[subNode] = std::nanf("");
		m_bvh4Nodes[bvh4Node].m_aabbMinZ[subNode] = std::nanf("");

		m_bvh4Nodes[bvh4Node].m_aabbMaxX[subNode] = std::nanf("");
		m_bvh4Nodes[bvh4Node].m_aabbMaxY[subNode] = std::nanf("");
		m_bvh4Nodes[bvh4Node].m_aabbMaxZ[subNode] = std::nanf("");

		m_bvh4Nodes[bvh4Node].m_validity[subNode] = 0;
	}

	return bvh4Node;
}

// --------------------------------------------------------------------------------
const BVH4Node& BVH4AccellStructure::GetBVH4Node(const uint32_t index) const
{
	assert(index < m_bvh4Nodes.size());
	return m_bvh4Nodes[index];
}

// --------------------------------------------------------------------------------
const MaterialIndex4& BVH4AccellStructure::GetMaterialIndex4(const uint32_t index) const
{
	assert(index < (uint32_t)m_materialIndex4s.size());
	return m_materialIndex4s[index];
}

// --------------------------------------------------------------------------------
const TraversalTriangle4& BVH4AccellStructure::GetTraversalTriangle4(const uint32_t index) const
{
	assert(index < (uint32_t)m_traversalTriangle4s.size());
	return m_traversalTriangle4s[index];
}

// --------------------------------------------------------------------------------
const TriangleIndex4& BVH4AccellStructure::GetTriangleIndex4(const uint32_t index) const
{
	assert(index < (uint32_t)m_triangleIndex4s.size());
	return m_triangleIndex4s[index];
}

// --------------------------------------------------------------------------------
const TriangleTexCoord4& BVH4AccellStructure::GetTriangleTexCoord4(const uint32_t index) const
{
	assert(index < (uint32_t)m_triangleTexCoord4s.size());
	return m_triangleTexCoord4s[index];
}
