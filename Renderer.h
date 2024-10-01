#pragma once

#include<cstdint>
#include <vector>
#include "Camera.h"
#include "Vector3.h"
#include "RGB.h"
#include "HitResult.h"

class Framebuffer;

class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void UpdateFramebufferContents(Framebuffer* framebuffer);

private:


	void HitSphere(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax, HitResult& hitResult);
	void HitPlane(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax, HitResult& hitResult);
	HitResult TraceRay(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax);

	std::vector<Vector3> m_sphereList;
	std::vector<RGB> m_sphereColours;

	Camera m_camera;
};

