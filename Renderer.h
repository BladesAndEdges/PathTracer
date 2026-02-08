#pragma once

#include<cstdint>
#include<vector>

#include "Camera.h"
#include "HitResult.h"
#include "Ray.h"
#include "Triangle.h"
#include "TraversalTriangle4.h"
#include "ViewportDesc.h"

class BVH2AccellStructure;
class BVH4AccellStructure;
class Framebuffer;
struct Material4Index;
class PerformanceCounter;
class SceneManager;
class TraversalDataManager;
struct TraversalTriangle;
struct TriangleIndices;
struct TriangleTexCoords4;

// --------------------------------------------------------------------------------
class Renderer
{
public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc);

private:

	void RegenerateViewSpaceDirections(Framebuffer* framebuffer);

	template<bool T_acceptAnyHit>
	void HitTriangles4(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	Vector3 PathTrace(Ray& ray, const uint32_t rayIndex, uint32_t depth);

	template<bool T_acceptAnyHit>
	HitResult TraceRayNonBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	HitResult TraceRay4NonBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	TraversalDataManager* m_traversalDataManager;
	SceneManager* m_sceneManager;

	template<bool T_acceptAnyHit>
	void TraverseBVH2(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void BVH2DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH2(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void HitTriangle(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const uint32_t triangleIndex, HitResult& out_hitResult, bool& out_hasHit);

	__forceinline void HitTriangleV2(Ray& ray, const TraversalTriangle& traversalTriangle, const uint32_t primitiveId, const float tMin, uint32_t& out_primitiveId,
		float& out_tMax, float& out_u, float& out_v, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	void BVH4HitTriangle4(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const TriangleIndices& triangleIndices, const TraversalTriangle4& triangle4, 
		const Material4Index& material4Index, const TriangleTexCoords4& triangleTexCoords4, HitResult& out_hitResult, bool& out_hasHit);

	// BVH4 code
	template<bool T_acceptAnyHit>
	void TraverseBVH4(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void BVH4DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin);
};

