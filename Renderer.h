#pragma once

#include<cstdint>
#include <vector>
#include "Camera.h"
#include "Vector3.h"
#include "RGB.h"
#include "HitResult.h"
#include "Ray.h"
#include "ViewportDesc.h"

class Framebuffer;

struct sphereDiscriminantAndT
{
	float m_discriminant;
	float m_t;
};

class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized);

private:

	void RegenerateViewSpaceDirections(Framebuffer* framebuffer);
	template<bool T_acceptAnyHit>
	void HitSphere(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult);
	void HitPlane(const Ray& ray, const float tMin, float& tMax, const float distance, const Vector3& normalizedPlaneNormal, RGB colour, HitResult& out_hitResult);

	template<bool T_acceptAnyHit>
	HitResult TraceRay(const Ray& ray, const float tMin);

	std::vector<Vector3> m_sphereList;
	std::vector<RGB> m_sphereColours;

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;

	RGB c_indigo;

	// Big tesst
	std::vector<sphereDiscriminantAndT> m_sphereDiscriminantAndTValues;
};

