#pragma once

#include<cstdint>
#include<vector>

#include "Camera.h"
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

	std::vector<float> m_sphereRadii;

	// Positional data of spheres
	std::vector<float> m_sphereCentersX;
	std::vector<float> m_sphereCentersY;
	std::vector<float> m_sphereCentersZ;

	// Colour data of spheres
};

