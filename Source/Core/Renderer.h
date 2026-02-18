#pragma once

#include<cstdint>
#include<vector>

#include "Camera.h"
#include "HitResult.h"
#include "Ray.h"
#include "ViewportDesc.h"

class BVH2AccellStructure;
class BVH4AccellStructure;
class Framebuffer;
class PerformanceCounter;
class SceneManager;
class TraversalDataManager;

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

	Vector3 PathTrace(Ray& ray, const uint32_t rayIndex, uint32_t depth);

	// Non-BVH Scalar
	template<bool T_acceptAnyHit>
	HitResult TraceRayNonBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	// Non-BVH SSE
	template<bool T_acceptAnyHit>
	HitResult TraceRay4NonBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	// BVH2
	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH2(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void BVH2DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, uint32_t& out_primitiveId,
		float& out_tMax, float& out_u, float& out_v, bool& out_hasHit);

	// BVH4
	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void BVH4DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, __m128i& out_primitiveId, __m128& out_tMax, __m128& out_u, __m128& out_v, int& moveMask);

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	TraversalDataManager* m_traversalDataManager;
	SceneManager* m_sceneManager;
};

