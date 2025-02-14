#pragma once

#include<cstdint>
#include<vector>

#include "Camera.h"
#include "HitResult.h"
#include "Ray.h"
#include "Triangle.h"
#include "Triangle4.h"
#include "ViewportDesc.h"

class BVHAccellStructure;
class Framebuffer;

class Renderer
{

public:

	Renderer(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ,
		const std::vector<Triangle4>& triangle4s, const std::vector<Triangle>& faces, const Vector3& center);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized);

private:

	void RegenerateViewSpaceDirections(Framebuffer* framebuffer);
	template<bool T_acceptAnyHit>
	void HitSphere(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult);
	void HitPlane(const Ray& ray, const float tMin, float& tMax, const float distance, const Vector3& normalizedPlaneNormal, Vector3 colour, HitResult& out_hitResult);
	void HitQuad(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult);
	bool IsInsideQuad(const float alpha, const float beta);

	// Debug build contains an extra ray index paramater
	template<bool T_acceptAnyHit>
	void HitTriangles(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	Vector3 PathTrace(Ray& ray, const uint32_t rayIndex,uint32_t depth);

	template<bool T_acceptAnyHit>
	HitResult TraceRay(Ray& ray, const uint32_t rayIndex, const float tMin);

	std::vector<Vector3> m_sphereList;
	std::vector<Vector3> m_sphereColours;

	std::vector<Vector3> m_quadList;
	std::vector<Vector3> m_quadColours;
	std::vector<Vector3> m_quadUs;
	std::vector<Vector3> m_quadVs;

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	Vector3 c_indigo;
	Vector3 c_white;
	Vector3 c_grey;

	std::vector<float> m_sphereRadii;

	// Positional data of spheres
	std::vector<float> m_sphereCentersX;
	std::vector<float> m_sphereCentersY;
	std::vector<float> m_sphereCentersZ;

	std::vector<Triangle> m_faces;
	Vector3 m_center;

	// SSE for triangles
	std::vector<Triangle4> m_triangle4s;

	// Scalar for triangles
	std::vector<float> m_positionsX;
	std::vector<float> m_positionsY;
	std::vector<float> m_positionsZ;

	BVHAccellStructure* m_bvhAccellStructure;

	template<bool T_acceptAnyHit>
	void TraverseBVH(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	void DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit);

	template<bool T_acceptAnyHit>
	HitResult TraceRayAgainstBVH(Ray& ray, const uint32_t rayIndex, const float tMin);

	template<bool T_acceptAnyHit>
	void HitTriangle(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const uint32_t triangleIndex, HitResult& out_hitResult, bool& out_hasHit);
};

