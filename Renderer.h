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
class Framebuffer;
class PerformanceCounter;

class Renderer
{

public:

	Renderer(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ,
		const std::vector<Triangle4>& triangle4s, const std::vector<Triangle>& faces, const Vector3& center);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc);

private:

	void RegenerateViewSpaceDirections(Framebuffer* framebuffer);

	// Debug build contains an extra ray index paramater
	template<bool T_acceptAnyHit>
	void HitTriangles(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	Vector3 PathTrace(Ray& ray, const uint32_t rayIndex,uint32_t depth);

	template<bool T_acceptAnyHit>
	HitResult TraceRay(Ray& ray, const uint32_t rayIndex, const float tMin);


	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;


	std::vector<Triangle> m_faces;
	Vector3 m_center;

	// SSE for triangles
	std::vector<Triangle4> m_triangle4s;

	// Scalar for triangles
	std::vector<float> m_positionsX;
	std::vector<float> m_positionsY;
	std::vector<float> m_positionsZ;

	BVH2AccellStructure* m_bvhAccellStructure;

	template<bool T_acceptAnyHit>
	void TraverseBVH(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceRayAgainstBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void HitTriangle(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const uint32_t triangleIndex, HitResult& out_hitResult, bool& out_hasHit);
};

