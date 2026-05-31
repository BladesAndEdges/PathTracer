#include "Pathtracer.h"

#include "Camera.h"
#include "Framebuffer.h"
#include "HitResult.h"
#include "Intersections.h"
#include "Material4Index.h"
#include "Ray.h"
#include "SceneManager.h"
#include "Traversals.h"
#include "TraversalDataManager.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"
#include "TriangleTexCoords4.h"

# define M_PI 3.14159265358979323846
# define TMIN 1e-5f

//#define TRACE_AGAINST_NON_BVH
//#define TRACE_AGAINST_NON_BVH_SSE
//#define TRACE_AGAINST_BVH2
//#define TRACE_AGAINST_BVH4

// --------------------------------------------------------------------------------
Pathtracer::Pathtracer() : m_pixelWidth(0.0f), m_pixelHeight(0.0f)
{
}

// --------------------------------------------------------------------------------
void Pathtracer::CreateImage()
{
}

// --------------------------------------------------------------------------------
void Pathtracer::GenerateViewspaceDirections(const uint32_t framebufferWidth, const uint32_t framebufferHeight)
{
	m_viewspaceDirections.clear();

	const float aspectRatio = (float)framebufferWidth / (float)framebufferHeight;
	const float width = 2.0f;
	const float height = width / aspectRatio;
	const float halfFov = 45.0f * ((float)M_PI / 180.0f);
	const float distanceToPlane = (width / 2.0f) / tanf(halfFov);

	m_pixelWidth = width / framebufferWidth;
	m_pixelHeight = height / framebufferHeight;

	// Corners of the plane
	const Vector3 topLeft(-width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomLeft(-width / 2.0f, -height / 2.0f, -distanceToPlane);
	const Vector3 topRight(width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomRight(width / 2.0f, -height / 2.0f, -distanceToPlane);

	for (uint32_t row = 0u; row < framebufferHeight; row++)
	{
		const float ty = (row * m_pixelHeight) / height;
		const Vector3 r0 = (1.0f - ty) * topLeft + ty * bottomLeft;
		const Vector3 r1 = (1.0f - ty) * topRight + ty * bottomRight;

		for (uint32_t column = 0u; column < framebufferWidth; column++)
		{
			const float tx = (column * m_pixelWidth) / width;

			Vector3 texelCenter = (1.0f - tx) * r0 + tx * r1;
			const float xVal = texelCenter.X() + (m_pixelWidth / 2.0f);

			const float valDegreesToRad = 90.0f * ((float)M_PI / 180.0f);
			const float rotatedX = cosf(valDegreesToRad) * xVal + sinf(valDegreesToRad) * texelCenter.Z();
			const float rotatedZ = -sinf(valDegreesToRad) * xVal + cosf(valDegreesToRad) * texelCenter.Z();

			texelCenter.SetX(rotatedX);
			texelCenter.SetY(texelCenter.Y() - (m_pixelHeight / 2.0f));
			texelCenter.SetZ(rotatedZ);
			m_viewspaceDirections.push_back(texelCenter);
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderSurfaceColour(Camera* camera, Framebuffer* framebuffer, 
	const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, uint8_t* out_pixels)
{
	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + 
				(column * framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			const HitResult hitResult = ScalarTraceRay<true>(traversalDataManager, sceneManager, ray);

			out_pixels[byteIndex] = uint8_t((hitResult.m_colour.X() * 255.0f) + 0.5f);
			out_pixels[byteIndex + 1u] = uint8_t((hitResult.m_colour.Y() * 255.0f) + 0.5f);
			out_pixels[byteIndex + 2u] = uint8_t((hitResult.m_colour.Z() * 255.0f) + 0.5f);
			out_pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::ScalarTraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	const std::vector<TraversalTriangle>& traversalTriangles = traversalDataManager->GetTraversalTriangles();
	const TraversalTriangle* const beginTriangle = &traversalTriangles[0u];
	const TraversalTriangle* const endTriangle = beginTriangle + traversalTriangles.size();

	const std::vector<uint32_t>& materialIndices = traversalDataManager->GetMaterialIndices();
	const std::vector<TriangleTexCoords>& triangleTexCoords = traversalDataManager->GetTriangleTexCoords();

	uint32_t triangleIndex = 0u;
	for (const TraversalTriangle* triangle = beginTriangle; triangle != endTriangle; triangle++)
	{
		HitTriangle(ray, *triangle, triangleIndex, TMIN, primitiveId, tMax, tu, tv, hasHit);
		
		if (T_acceptAnyHit)
		{
			if (hasHit)
			{
				break;
			}
		}

		triangleIndex++;
	}

	if (hasHit)
	{
		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

		const TriangleTexCoords& texCoords = triangleTexCoords[primitiveId];
		hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoords.m_v0uv[0u] + tu * texCoords.m_v1uv[0u] + tv * texCoords.m_v2uv[0u]);
		hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoords.m_v0uv[1u] + tu * texCoords.m_v1uv[1u] + tv * texCoords.m_v2uv[1u]);

		const Vector3 edge1 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge1[0u], traversalTriangles[primitiveId].m_edge1[1u], traversalTriangles[primitiveId].m_edge1[2u]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge2[0u], traversalTriangles[primitiveId].m_edge2[1u], traversalTriangles[primitiveId].m_edge2[2u]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = materialIndices[primitiveId];

		hitResult.m_colour = sceneManager->BasicSample(materialIndices[primitiveId], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::SSETraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;

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
	const __m128 tMinimum = _mm_set1_ps(TMIN);

	// Loop outputs
	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);

	const std::vector<TraversalTriangle4>& traversalTriangle4s = traversalDataManager->GetTraversalTriangle4s();
	const TraversalTriangle4* const beginTriangle4 = &traversalTriangle4s[0u];
	const TraversalTriangle4* const endTriangle4 = beginTriangle4 + traversalTriangle4s.size();

	int currentIndex = 0u;
	int moveMask = 0u;
	for (const TraversalTriangle4* triangle4 = beginTriangle4; triangle4 != endTriangle4; triangle4++)
	{
		HitTriangle4(ray, *triangle4, currentIndex, TMIN, outTri4Indices, outTMax, outU, outV, moveMask);

		if (T_acceptAnyHit)
		{
			if (moveMask)
			{
				break;
			}
		}

		currentIndex++;
	}

	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));

	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));

	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));

	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));

	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);

	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];

		float us[4u];
		_mm_storeu_ps(us, outU);
		float u = us[subIndex];

		float vs[4u];
		_mm_storeu_ps(vs, outV);
		float v = vs[subIndex];

		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

		const std::vector<TriangleTexCoords4>& texCoords4 = traversalDataManager->GetTriangleTexCoords4();
		hitResult.m_texCoords.SetX((1.0f - u - v) * texCoords4[tri4Index].m_v0U[subIndex] + u * texCoords4[tri4Index].m_v1U[subIndex] + v * texCoords4[tri4Index].m_v2U[subIndex]);
		hitResult.m_texCoords.SetY((1.0f - u - v) * texCoords4[tri4Index].m_v0V[subIndex] + u * texCoords4[tri4Index].m_v1V[subIndex] + v * texCoords4[tri4Index].m_v2V[subIndex]);

		const Vector3 edge1 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge1X[subIndex],
			traversalTriangle4s[tri4Index].m_edge1Y[subIndex],
			traversalTriangle4s[tri4Index].m_edge1Z[subIndex]));

		const Vector3 edge2 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge2X[subIndex],
			traversalTriangle4s[tri4Index].m_edge2Y[subIndex],
			traversalTriangle4s[tri4Index].m_edge2Z[subIndex]));

		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = (tri4Index * 4u) + subIndex;

		const std::vector<Material4Index>& material4Indices = traversalDataManager->GetMaterial4Indices();
		hitResult.m_materialId = material4Indices[tri4Index].m_indices[subIndex];

		hitResult.m_colour = sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::BVH2TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	BVH2Traversal<T_acceptAnyHit>(traversalDataManager, 0u, ray, TMIN, primitiveId, tMax, tu, tv, hasHit);

	if (hasHit)
	{
		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);


		const TriangleTexCoords& texCoords = traversalDataManager->GetBVH2TriangleTexCoords(primitiveId);
		hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoords.m_v0uv[0u] + tu * texCoords.m_v1uv[0u] + tv * texCoords.m_v2uv[0u]);
		hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoords.m_v0uv[1u] + tu * texCoords.m_v1uv[1u] + tv * texCoords.m_v2uv[1u]);

		const TraversalTriangle& traversalTriangle = traversalDataManager->GetBVH2TraversalTriangle(primitiveId);
		const Vector3 edge1 = Normalize(Vector3(traversalTriangle.m_edge1[0u], traversalTriangle.m_edge1[1u], traversalTriangle.m_edge1[2u]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangle.m_edge2[0u], traversalTriangle.m_edge2[1u], traversalTriangle.m_edge2[2u]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = traversalDataManager->GetBVH2MaterialIndex(primitiveId);

		hitResult.m_colour = sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::BVH4TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;

	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);
	int moveMask = 0;
	
	BVH4Traversal<T_acceptAnyHit>(traversalDataManager, 0u, ray, TMIN, outTri4Indices, outTMax, outU, outV, moveMask);
	
	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));
	
	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));
	
	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));
	
	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));
	
	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);
	
	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];
	
		float us[4u];
		_mm_storeu_ps(us, outU);
		float u = us[subIndex];
	
		float vs[4u];
		_mm_storeu_ps(vs, outV);
		float v = vs[subIndex];
	
		hitResult.m_t = tMax;
	
		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);
	
		const TriangleTexCoords4& triangleTexCoords4 = traversalDataManager->GetBVH4TriangleTexCoords4(tri4Index);
		hitResult.m_texCoords.SetX((1.0f - u - v) * triangleTexCoords4.m_v0U[subIndex] + u * triangleTexCoords4.m_v1U[subIndex] + v * triangleTexCoords4.m_v2U[subIndex]);
		hitResult.m_texCoords.SetY((1.0f - u - v) * triangleTexCoords4.m_v0V[subIndex] + u * triangleTexCoords4.m_v1V[subIndex] + v * triangleTexCoords4.m_v2V[subIndex]);
	
		const TraversalTriangle4& traversalTriangle4 = traversalDataManager->GetBVH4TraversalTriangle4(tri4Index);
		const Vector3 edge1 = Normalize(Vector3(traversalTriangle4.m_edge1X[subIndex], traversalTriangle4.m_edge1Y[subIndex], traversalTriangle4.m_edge1Z[subIndex]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangle4.m_edge2X[subIndex], traversalTriangle4.m_edge2Y[subIndex], traversalTriangle4.m_edge2Z[subIndex]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;
	
		const TriangleIndices& triangleIndices = traversalDataManager->GetBVH4TriangleIndices(tri4Index);
		hitResult.m_primitiveId = triangleIndices.m_triangleIndices[subIndex];
	
		const Material4Index& material4Index = traversalDataManager->GetBVH4Material4Index(tri4Index);
		hitResult.m_materialId = material4Index.m_indices[subIndex];
	
		hitResult.m_colour = sceneManager->BasicSample(material4Index.m_indices[subIndex], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
const std::vector<Vector3>& Pathtracer::GetViewSpaceDirections() const
{
	return m_viewspaceDirections;
}

// --------------------------------------------------------------------------------
float Pathtracer::GetPixelWidth() const
{
	return m_pixelWidth;
}

// --------------------------------------------------------------------------------
float Pathtracer::GetPixelHeight() const
{
	return m_pixelHeight;
}

// --------------------------------------------------------------------------------
template HitResult Pathtracer::ScalarTraceRay<true>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);
template HitResult Pathtracer::ScalarTraceRay<false>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);

template HitResult Pathtracer::SSETraceRay<true>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);
template HitResult Pathtracer::SSETraceRay<false>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);

template HitResult Pathtracer::BVH2TraceRay<true>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);
template HitResult Pathtracer::BVH2TraceRay<false>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);

template HitResult Pathtracer::BVH4TraceRay<true>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);
template HitResult Pathtracer::BVH4TraceRay<false>(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray);