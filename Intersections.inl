#include "Ray.h"
#include "TraversalTriangle.h"
#include "TraversalTriangle4.h"
#include "Vector3.h"

// --------------------------------------------------------------------------------
__forceinline void HitTriangle(Ray& ray, const TraversalTriangle& traversalTriangle, const uint32_t primitiveId, const float tMin, uint32_t& out_primitiveId,
	float& out_tMax, float& out_u, float& out_v, bool& out_hasHit)
{
	const Vector3 edge1 = Vector3(traversalTriangle.m_edge1[0u], traversalTriangle.m_edge1[1u], traversalTriangle.m_edge1[2u]);
	const Vector3 edge2 = Vector3(traversalTriangle.m_edge2[0u], traversalTriangle.m_edge2[1u], traversalTriangle.m_edge2[2u]);

	// Cross product will approach 0s as the directions start facing the same way, or opposite (so parallel)
	const Vector3 pVec = Cross(ray.Direction(), edge2);
	const float det = Dot(pVec, edge1);

	if (std::fabs(det) >= 1e-8f)
	{
		const float invDet = 1.0f / det;

		const Vector3 tVec = ray.Origin() - Vector3(traversalTriangle.m_v0[0u],
			traversalTriangle.m_v0[1u],
			traversalTriangle.m_v0[2u]);

		const float u = Dot(tVec, pVec) * invDet;

		if ((u >= 0.0f) && (u <= 1.0f))
		{
			const Vector3 qVec = Cross(tVec, edge1);
			const float v = Dot(ray.Direction(), qVec) * invDet;

			if ((v >= 0.0f) && ((u + v) <= 1.0f))
			{
				const float t = Dot(edge2, qVec) * invDet;

				if ((t >= tMin) && (t <= out_tMax))
				{
					out_primitiveId = primitiveId;
					out_tMax = t;
					out_u = u;
					out_v = v;
					out_hasHit = true;
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
void HitTriangle4(Ray& ray, const TraversalTriangle4& traversalTriangle4, const int triangle4, const float tMin,
	__m128i& out_triangle4, __m128& out_tMax, __m128& out_u, __m128& out_v, int& moveMask)
{
	// Constants
	const __m128 epsilon = _mm_set1_ps(1e-8f);
	const __m128 zeros = _mm_set1_ps(0.0f);
	const __m128 ones = _mm_set1_ps(1.0f);

	// Ray data
	const __m128 rayOriginX = _mm_set1_ps(ray.Origin().X());
	const __m128 rayOriginY = _mm_set1_ps(ray.Origin().Y());
	const __m128 rayOriginZ = _mm_set1_ps(ray.Origin().Z());

	const __m128 rayDirectionX = _mm_set1_ps(ray.Direction().X());
	const __m128 rayDirectionY = _mm_set1_ps(ray.Direction().Y());
	const __m128 rayDirectionZ = _mm_set1_ps(ray.Direction().Z());

	// tMin
	const __m128 tMinimum = _mm_set1_ps(tMin);

	// Triangle4 data
	const __m128 edge1X = _mm_loadu_ps(traversalTriangle4.m_edge1X);
	const __m128 edge1Y = _mm_loadu_ps(traversalTriangle4.m_edge1Y);
	const __m128 edge1Z = _mm_loadu_ps(traversalTriangle4.m_edge1Z);

	const __m128 edge2X = _mm_loadu_ps(traversalTriangle4.m_edge2X);
	const __m128 edge2Y = _mm_loadu_ps(traversalTriangle4.m_edge2Y);
	const __m128 edge2Z = _mm_loadu_ps(traversalTriangle4.m_edge2Z);

	const __m128 v0X = _mm_loadu_ps(traversalTriangle4.m_v0X);
	const __m128 v0Y = _mm_loadu_ps(traversalTriangle4.m_v0Y);
	const __m128 v0Z = _mm_loadu_ps(traversalTriangle4.m_v0Z);

	// TRY TO USE FUSED MULTIPLY SUBTRACT IF IT EXISTS HERE

	// Calculate pVec
	const __m128 pvecXLHS = _mm_mul_ps(rayDirectionY, edge2Z);
	const __m128 pvecXRHS = _mm_mul_ps(rayDirectionZ, edge2Y);
	const __m128 pvecX = _mm_sub_ps(pvecXLHS, pvecXRHS);

	const __m128 pvecYLHS = _mm_mul_ps(rayDirectionZ, edge2X);
	const __m128 pvecYRHS = _mm_mul_ps(rayDirectionX, edge2Z);
	const __m128 pvecY = _mm_sub_ps(pvecYLHS, pvecYRHS);

	const __m128 pvecZLHS = _mm_mul_ps(rayDirectionX, edge2Y);
	const __m128 pvecZRHS = _mm_mul_ps(rayDirectionY, edge2X);
	const __m128 pvecZ = _mm_sub_ps(pvecZLHS, pvecZRHS);

	// Calculate determinants
	const __m128 detDotX = _mm_mul_ps(pvecX, edge1X); // fmadd: _mm_fmadd_ps(), header is already added
	const __m128 detDotY = _mm_mul_ps(pvecY, edge1Y);
	const __m128 detDotZ = _mm_mul_ps(pvecZ, edge1Z);

	const __m128 detAddXY = _mm_add_ps(detDotX, detDotY);
	const __m128 determinants = _mm_add_ps(detAddXY, detDotZ);

	const __m128 signMask = _mm_set1_ps(-0.0f);
	const __m128 absDeterminants = _mm_andnot_ps(signMask, determinants);

	// Calculate intersection mask
	const __m128 hasIntersectedMask = _mm_cmpge_ps(absDeterminants, epsilon); // mask

	// Calculate inverse determinant
	const __m128 invDeterminant = _mm_div_ps(ones, determinants);

	// Calculate tvec
	const __m128 tvecX = _mm_sub_ps(rayOriginX, v0X);
	const __m128 tvecY = _mm_sub_ps(rayOriginY, v0Y);
	const __m128 tvecZ = _mm_sub_ps(rayOriginZ, v0Z);

	// Calculate u
	const __m128 uDotX = _mm_mul_ps(tvecX, pvecX);
	const __m128 uDotY = _mm_mul_ps(tvecY, pvecY);
	const __m128 uDotZ = _mm_mul_ps(tvecZ, pvecZ);

	const __m128 uAddXY = _mm_add_ps(uDotX, uDotY);
	const __m128 uAddFinal = _mm_add_ps(uAddXY, uDotZ);
	const __m128 u = _mm_mul_ps(uAddFinal, invDeterminant);

	// Calculate u mask
	const __m128 uIsGreaterEqual0 = _mm_cmpge_ps(u, zeros);
	const __m128 uIsLessEqual1 = _mm_cmple_ps(u, ones);
	const __m128 isUValidMask = _mm_and_ps(uIsGreaterEqual0, uIsLessEqual1); // mask

	// Calculate qvec
	const __m128 qvecXLHS = _mm_mul_ps(tvecY, edge1Z);
	const __m128 qvecXRHS = _mm_mul_ps(tvecZ, edge1Y);
	const __m128 qvecX = _mm_sub_ps(qvecXLHS, qvecXRHS);

	const __m128 qvecYLHS = _mm_mul_ps(tvecZ, edge1X);
	const __m128 qvecYRHS = _mm_mul_ps(tvecX, edge1Z);
	const __m128 qvecY = _mm_sub_ps(qvecYLHS, qvecYRHS);

	const __m128 qvecZLHS = _mm_mul_ps(tvecX, edge1Y);
	const __m128 qvecZRHS = _mm_mul_ps(tvecY, edge1X);
	const __m128 qvecZ = _mm_sub_ps(qvecZLHS, qvecZRHS);

	// Calculate v
	const __m128 vDotX = _mm_mul_ps(rayDirectionX, qvecX);
	const __m128 vDotY = _mm_mul_ps(rayDirectionY, qvecY);
	const __m128 vDotZ = _mm_mul_ps(rayDirectionZ, qvecZ);

	const __m128 vAddXY = _mm_add_ps(vDotX, vDotY);
	const __m128 vAddFinal = _mm_add_ps(vAddXY, vDotZ);
	const __m128 v = _mm_mul_ps(vAddFinal, invDeterminant);

	// Calculate v masks
	const __m128 vIsGreaterEqual0 = _mm_cmpge_ps(v, zeros);

	const __m128 uAddV = _mm_add_ps(u, v);
	const __m128 uAddVLessEqual1 = _mm_cmple_ps(uAddV, ones);

	const __m128 isInsideTriangleMask = _mm_and_ps(vIsGreaterEqual0, uAddVLessEqual1); // mask

	// Calculate t
	const __m128 tDotX = _mm_mul_ps(edge2X, qvecX);
	const __m128 tDotY = _mm_mul_ps(edge2Y, qvecY);
	const __m128 tDotZ = _mm_mul_ps(edge2Z, qvecZ);

	const __m128 tAddXY = _mm_add_ps(tDotX, tDotY);
	const __m128 tAddFinal = _mm_add_ps(tAddXY, tDotZ);
	const __m128 t = _mm_mul_ps(tAddFinal, invDeterminant);

	// Between tMin and tMax masks
	const __m128 tMoreThanTMin = _mm_cmpge_ps(t, tMinimum);
	const __m128 tLessThanTMax = _mm_cmple_ps(t, out_tMax);
	const __m128 tCheckMask = _mm_and_ps(tMoreThanTMin, tLessThanTMax);

	// Validity mask
	const __m128 tValidityMask = _mm_and_ps(_mm_and_ps(_mm_and_ps(hasIntersectedMask, isUValidMask),
		isInsideTriangleMask), tCheckMask);

	// Valid t values
	out_tMax = _mm_or_ps(_mm_and_ps(tValidityMask, t),
		_mm_andnot_ps(tValidityMask, out_tMax));

	// Triangle4 index
	const __m128i tri4Index = _mm_set_epi32(triangle4, triangle4, triangle4, triangle4);
	out_triangle4 = _mm_or_epi32(_mm_and_epi32(_mm_castps_si128(tValidityMask), tri4Index),
		_mm_andnot_epi32(_mm_castps_si128(tValidityMask), out_triangle4));

	// Triangle u values
	out_u = _mm_or_ps(_mm_and_ps(tValidityMask, u),
		_mm_andnot_ps(tValidityMask, out_u));

	// Triangle v values
	out_v = _mm_or_ps(_mm_and_ps(tValidityMask, v),
		_mm_andnot_ps(tValidityMask, out_v));

	moveMask = _mm_movemask_ps(tValidityMask);
}