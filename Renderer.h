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


class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized);

private:


	void HitSphere(const Ray& ray, const float tMin, const float tMax, HitResult& hitResult);
	void HitPlane(const Ray& ray, const float tMin, const float tMax, HitResult& hitResult);
	HitResult TraceRay(const Ray& ray, const float tMin, const float tMax);

	std::vector<Vector3> m_sphereList;
	std::vector<RGB> m_sphereColours;

	Camera m_camera;
	Vector3 m_lightDirection;
	ViewportDesc m_viewportDesc;

	std::vector<Vector3> m_texelCenters;
	bool m_isFirstFrame;
};

