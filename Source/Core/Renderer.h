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

	// BVH4
	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin);

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	TraversalDataManager* m_traversalDataManager;
	SceneManager* m_sceneManager;
};

