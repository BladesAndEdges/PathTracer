#pragma once

#include <immintrin.h>
#include <stdint.h>

class Ray;
struct TraversalTriangle;
struct TraversalTriangle4;

// --------------------------------------------------------------------------------
__forceinline void HitTriangle(Ray& ray, const TraversalTriangle& traversalTriangle, const uint32_t primitiveId, const float tMin, uint32_t& out_primitiveId,
	float& out_tMax, float& out_u, float& out_v, bool& out_hasHit);

// --------------------------------------------------------------------------------
void HitTriangle4(Ray& ray, const TraversalTriangle4& traversalTriangle4, const int triangle4, const float tMin,
	__m128i& out_triangle4, __m128& out_tMax, __m128& out_u, __m128& out_v, int& moveMask);

#include "Intersections.inl"