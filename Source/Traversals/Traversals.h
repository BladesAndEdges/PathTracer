#pragma once

#include <stdint.h>

#include "BVHNode.h"
#include "BVH4Node.h"
#include "Ray.h"
#include "TraversalDataManager.h"

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void BVH2Traversal(const TraversalDataManager* dataManager, const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, uint32_t& out_primitiveId,
	float& out_tMax, float& out_u, float& out_v, bool& out_hasHit)
{
	if (!T_acceptAnyHit)
	{
		ray.m_primaryNodeVisits++;
	}

	const BVH2InnerNode& node = dataManager->GetBVH2InnerNode(innerNodeStartIndex);

	float tNears[2u] = { INFINITY, INFINITY };
	float hit[2u] = { false, false };
	hit[0u] = RayAABBIntersection(ray, T_acceptAnyHit, node.m_leftAABB.m_min.X(), node.m_leftAABB.m_min.Y(), node.m_leftAABB.m_min.Z(),
		node.m_leftAABB.m_max.X(), node.m_leftAABB.m_max.Y(), node.m_leftAABB.m_max.Z(), out_tMax, &tNears[0u]);
	hit[1u] = RayAABBIntersection(ray, T_acceptAnyHit, node.m_rightAABB.m_min.X(), node.m_rightAABB.m_min.Y(), node.m_rightAABB.m_min.Z(),
		node.m_rightAABB.m_max.X(), node.m_rightAABB.m_max.Y(), node.m_rightAABB.m_max.Z(), out_tMax, &tNears[0u]);

	if (!hit[0u] && !hit[1u])
	{
		return;
	}

	// Reorder if needed
	uint32_t visitOrder[2u] = { node.m_leftChild, node.m_rightChild };
	AABB aabbs[2u] = { node.m_leftAABB, node.m_rightAABB };
	if (tNears[0u] > tNears[1u])
	{
		std::swap(visitOrder[0u], visitOrder[1u]);
		std::swap(hit[0u], hit[1u]);

		const AABB tempAABB = aabbs[0u];
		aabbs[0u] = aabbs[1u];
		aabbs[1u] = tempAABB;
	}

	// Traversal
	for (uint32_t child = 0u; child < 2u; child++)
	{
		if (!hit[child])
		{
			continue;
		}

		if (visitOrder[child] >> 31u)
		{
			const uint32_t triangleIndex = visitOrder[child] & ~(1u << 31u);
			const TraversalTriangle& traversalTriangle = dataManager->GetBVH2TraversalTriangle(triangleIndex);
			HitTriangle(ray, traversalTriangle, triangleIndex, tMin, out_primitiveId, out_tMax, out_u, out_v, out_hasHit);
		}
		else if (RayAABBIntersection(ray, T_acceptAnyHit, aabbs[child].m_min.X(), aabbs[child].m_min.Y(), aabbs[child].m_min.Z(),
			aabbs[child].m_max.X(), aabbs[child].m_max.Y(), aabbs[child].m_max.Z(), out_tMax, nullptr))
		{
			BVH2Traversal<T_acceptAnyHit>(dataManager, visitOrder[child], ray, tMin, out_primitiveId, out_tMax, out_u, out_v, out_hasHit);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (out_hasHit)
			{
				return;
			}
		}
	}
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void BVH4Traversal(const TraversalDataManager* dataManager, const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, __m128i& out_primitiveId, __m128& out_tMax, __m128& out_u, __m128& out_v, int& moveMask)
{
	if (!T_acceptAnyHit)
	{
		ray.m_primaryNodeVisits++;
	}

	const BVH4Node& node = dataManager->GetBVH4Node(innerNodeStartIndex);

#ifdef SORTED_BVH4

#if _DEBUG
	float theTNears[4u] = { INFINITY, INFINITY, INFINITY, INFINITY };
	int32_t theTNearsAsInts[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theTNearsAfterMask[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theTNearsAfterShiftLeft[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theCombinedTNearAndChildIndex[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
#endif

	uint32_t hitCount = 0u;
	int32_t nearPlaneAndChildIndex[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	for (uint32_t child = 0u; child < 4u; child++)
	{
		theTNearsAfterMask[child] = theTNearsAsInts[child] & 0x3FFFFFFE;
		theTNearsAfterShiftLeft[child] = theTNearsAfterMask[child] << 1;
		theCombinedTNearAndChildIndex[child] = theTNearsAfterShiftLeft[child] | child;

		float tNear = INFINITY;
		if (RayAABBIntersection(ray, T_acceptAnyHit, node.m_aabbMinX[child], node.m_aabbMinY[child], node.m_aabbMinZ[child], node.m_aabbMaxX[child],
			node.m_aabbMaxY[child], node.m_aabbMaxZ[child], tMax, &tNear))
		{
#if _DEBUG
			theTNears[child] = tNear;
			theTNearsAsInts[child] = *((int32_t*)&tNear);
			theTNearsAfterMask[child] = theTNearsAsInts[child] & 0x3FFFFFFE;
			theTNearsAfterShiftLeft[child] = theTNearsAfterMask[child] << 1;
			theCombinedTNearAndChildIndex[child] = theTNearsAfterShiftLeft[child] | child;
#endif
			nearPlaneAndChildIndex[child] = ((*(int32_t*)&tNear & 0x3FFFFFFE) << 1) | child;
			hitCount++;
		}
	}

	// Early out if no hit
	if (!hitCount)
	{
		return;
	}

	// Sort in ascending order
#if 1
	const int32_t a = std::min(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]);
	const int32_t b = std::max(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]);
	const int32_t c = std::min(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]);
	const int32_t d = std::max(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]);

	int32_t visitOrder[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	visitOrder[0u] = std::min(a, c);
	visitOrder[2u] = std::max(a, c);
	visitOrder[1u] = std::min(b, d);
	visitOrder[3u] = std::max(b, d);

	const int32_t e = std::min(visitOrder[1u], visitOrder[2u]);
	const int32_t f = std::max(visitOrder[1u], visitOrder[2u]);

	visitOrder[1u] = e;
	visitOrder[2u] = f;

#else
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[1u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]); }
	if (nearPlaneAndChildIndex[2u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[2u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[2u]); }
#endif

#ifdef _DEBUG
	//for (uint32_t visitNode = 0u; visitNode < 3u; visitNode++)
	//{
	//	assert(nearPlaneAndChildIndex[visitNode] <= nearPlaneAndChildIndex[visitNode + 1u]);
	//}
#endif

	//Traversal
	for (uint32_t child = 0u; child < hitCount; child++)
	{
#if 1
		const uint32_t visitIndex = (uint32_t)(visitOrder[child] & 0x00000003);
#else
		const uint32_t visitIndex = (uint32_t)nearPlaneAndChildIndex[child] & 0x00000003;
#endif
		if (node.m_child[visitIndex] >> 31u)
		{
			const uint32_t triangleIndex = node.m_child[visitIndex] & ~(1u << 31u);
			HitTriangle<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, triangleIndex, out_hitResult, out_hasHit);
		}
		else
		{
			BVH4DFSTraversal<T_acceptAnyHit>(node.m_child[visitIndex], ray, rayIndex, tMin, tMax, out_hitResult, out_hasHit);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (out_hasHit)
			{
				return;
			}
		}
	}
#endif

	// 0 andd tMax
	const __m128 zeroReg = _mm_set1_ps(0.0f);
	const __m128 tMaxReg = _mm_set1_ps(INFINITY); // Might want to add the actual tMax

	// Origin
	const __m128 originX = _mm_set1_ps(ray.Origin().X());
	const __m128 originY = _mm_set1_ps(ray.Origin().Y());
	const __m128 originZ = _mm_set1_ps(ray.Origin().Z());

	// Inverse direction
	const __m128 inverseDirX = _mm_set1_ps(ray.InverseDirection().X());
	const __m128 inverseDirY = _mm_set1_ps(ray.InverseDirection().Y());
	const __m128 inverseDirZ = _mm_set1_ps(ray.InverseDirection().Z());

	// AABB
	const __m128 minXs = _mm_loadu_ps(node.m_aabbMinX);
	const __m128 minYs = _mm_loadu_ps(node.m_aabbMinY);
	const __m128 minZs = _mm_loadu_ps(node.m_aabbMinZ);
	const __m128 maxXs = _mm_loadu_ps(node.m_aabbMaxX);
	const __m128 maxYs = _mm_loadu_ps(node.m_aabbMaxY);
	const __m128 maxZs = _mm_loadu_ps(node.m_aabbMaxZ);

	// Validity mask
	const __m128i validity = _mm_loadu_epi32(node.m_validity);

	// Calculate t0x and t1x
	const __m128 minSubOriginX = _mm_sub_ps(minXs, originX);
	const __m128 nX = _mm_mul_ps(minSubOriginX, inverseDirX);

	const __m128 maxSubOriginX = _mm_sub_ps(maxXs, originX);
	const __m128 fX = _mm_mul_ps(maxSubOriginX, inverseDirX);

	const __m128 nearX = _mm_min_ps(fX, nX);
	const __m128 farX = _mm_max_ps(nX, fX);

	const __m128 t0X = _mm_max_ps(nearX, zeroReg);
	const __m128 t1X = _mm_min_ps(farX, tMaxReg);

	// Calculate t0Y and t1Y
	const __m128 minSubOriginY = _mm_sub_ps(minYs, originY);
	const __m128 nY = _mm_mul_ps(minSubOriginY, inverseDirY);

	const __m128 maxSubOriginY = _mm_sub_ps(maxYs, originY);
	const __m128 fY = _mm_mul_ps(maxSubOriginY, inverseDirY);

	const __m128 nearY = _mm_min_ps(fY, nY);
	const __m128 farY = _mm_max_ps(nY, fY);

	const __m128 t0Y = _mm_max_ps(nearY, t0X);
	const __m128 t1Y = _mm_min_ps(farY, t1X);

	// Calculate t0Z and t1Z
	const __m128 minSubOriginZ = _mm_sub_ps(minZs, originZ);
	const __m128 nZ = _mm_mul_ps(minSubOriginZ, inverseDirZ);

	const __m128 maxSubOriginZ = _mm_sub_ps(maxZs, originZ);
	const __m128 fZ = _mm_mul_ps(maxSubOriginZ, inverseDirZ);

	const __m128 nearZ = _mm_min_ps(fZ, nZ);
	const __m128 farZ = _mm_max_ps(nZ, fZ);

	const __m128 t0Z = _mm_max_ps(nearZ, t0Y);
	const __m128 t1Z = _mm_min_ps(farZ, t1Y);

	// Check if an intersection occurred
	const __m128 hasIntersected = _mm_and_ps(_mm_cmple_ps(t0Z, t1Z), _mm_castsi128_ps(validity));
	const int intersectionMask = _mm_movemask_ps(hasIntersected);

	if (!intersectionMask)
	{
		return;
	}

	// Packing
	const __m128i int32Max = _mm_set1_epi32(INT32_MAX);
	const __m128i t0AsInts = _mm_castps_si128(_mm_or_ps(_mm_and_ps(hasIntersected, t0Z), _mm_andnot_ps(hasIntersected, _mm_castsi128_ps(int32Max))));
	const __m128i postChopBits = _mm_and_si128(t0AsInts, _mm_set1_epi32(0x3FFFFFFE));
	const __m128i shiftedLeft = _mm_slli_epi32(postChopBits, 1);

	const __m128i childIndicesInOrder = _mm_set_epi32(3, 2, 1, 0);
	const __m128i combinedT0AndIndex = _mm_or_epi32(shiftedLeft, childIndicesInOrder);

	// Sort
	const __m128i shuffle0 = _mm_shuffle_epi32(combinedT0AndIndex, _MM_SHUFFLE(1, 0, 1, 0));
	const __m128i min0 = _mm_min_epi32(combinedT0AndIndex, shuffle0);
	const __m128i max0 = _mm_max_epi32(combinedT0AndIndex, shuffle0);

	const __m128i shuffle1 = _mm_shuffle_epi32(max0, _MM_SHUFFLE(2, 3, 1, 0));;
	const __m128i min1 = _mm_min_epi32(min0, shuffle1);
	const __m128i max1 = _mm_max_epi32(min0, shuffle1);

	const __m128i shuffle2 = _mm_shuffle_epi32(max1, _MM_SHUFFLE(2, 3, 1, 0));
	const __m128i l3 = _mm_max_epi32(max1, shuffle2);
	const __m128i l2 = _mm_min_epi32(max1, shuffle2);

	const __m128i shuffle3 = _mm_shuffle_epi32(min1, _MM_SHUFFLE(2, 3, 1, 0));
	const __m128i l1 = _mm_max_epi32(min1, shuffle3);
	const __m128i l0 = _mm_min_epi32(min1, shuffle3);

	// Figure this out
	const __m128i unpack0 = _mm_unpackhi_epi32(l1, l3);
	const __m128i unpack1 = _mm_unpackhi_epi32(l0, l2);
	const __m128i result = _mm_unpackhi_epi32(unpack1, unpack0);

	const int visitOrderIndices[4u] =
	{
		_mm_cvtsi128_si32(result),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(1, 1, 1, 1))),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(3, 3, 3, 3)))
	};

	const uint32_t intersectionCount = __popcnt(*((uint32_t*)&intersectionMask));
	for (uint32_t i = 0u; i < intersectionCount; i++)
	{
		const uint32_t visitIndex = (uint32_t)(visitOrderIndices[i] & 0x00000003);

		if (node.m_child[visitIndex] >> 31u)
		{
			const uint32_t triangle4Index = node.m_child[visitIndex] & ~(1u << 31u);
			const TraversalTriangle4& triangle4 = dataManager->GetBVH4TraversalTriangle4(triangle4Index);

			HitTriangle4(ray, triangle4, triangle4Index, tMin, out_primitiveId, out_tMax, out_u, out_v, moveMask);
		}
		else
		{
			BVH4Traversal<T_acceptAnyHit>(dataManager, node.m_child[visitIndex], ray, tMin, out_primitiveId, out_tMax, out_u, out_v, moveMask);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (moveMask)
			{
				return;
			}
		}
	}
}