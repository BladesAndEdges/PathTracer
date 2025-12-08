#pragma once

#include<cstdint>
#include<vector>

#include "Camera.h"
#include "HitResult.h"
#include "Ray.h"
#include "Triangle.h"
#include "Triangle4.h"
#include "ViewportDesc.h"

class BVH2AccellStructure;
class BVH4AccellStructure;
class Framebuffer;
class PerformanceCounter;

class Renderer
{
public:

	Renderer(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ,
		const std::vector<Triangle4>& triangle4s, const std::vector<Triangle>& triangles, const Vector3& center);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc);

private:

	void RegenerateViewSpaceDirections(Framebuffer* framebuffer);

	// Debug build contains an extra ray index paramater
	template<bool T_acceptAnyHit>
	void HitTriangles(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	Vector3 PathTrace(Ray& ray, const uint32_t rayIndex, uint32_t depth);

	template<bool T_acceptAnyHit>
	HitResult TraceRay(Ray& ray, const uint32_t rayIndex, const float tMin);

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	std::vector<Triangle> m_triangles;
	Vector3 m_center;

	// SSE for triangles
	std::vector<Triangle4> m_triangle4s;

	// Scalar for triangles
	std::vector<float> m_positionsX;
	std::vector<float> m_positionsY;
	std::vector<float> m_positionsZ;

	BVH2AccellStructure* m_bvh2AccellStructure;
	BVH4AccellStructure* m_bvh4AccellStructure;

	template<bool T_acceptAnyHit>
	void TraverseBVH2(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void BVH2DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH2(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void HitTriangle(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const uint32_t triangleIndex, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	void BVH4HitTriangle4Scalar(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const Vector3& edge1, const Vector3& edge2, const Vector3& vertex0, 
		HitResult& out_hitResult, bool& out_hasHit);

	// BVH4 code
	template<bool T_acceptAnyHit>
	void TraverseBVH4(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void BVH4DFSTraversalWithTri4(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	void BVH4DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin);
};

