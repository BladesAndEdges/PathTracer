#pragma once

#include<cstdint>
#include <vector>
#include "Vector3.h"
#include "RGB.h"
#include "HitResult.h"

class Framebuffer;
class Ray;

class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void UpdateFramebufferContents(Framebuffer* framebuffer);

private:


	void HitSphere(const Vector3& texelCenter, HitResult& hitResult);
	void HitPlane(const Vector3& texelCenter, HitResult& hitResult);
	HitResult TraceRay(const Vector3& texelCenter);

	std::vector<Vector3> m_sphereList;
	std::vector<RGB> m_sphereColours;
};

