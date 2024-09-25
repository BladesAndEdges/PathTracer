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

	HitResult hitSphere(const Vector3& texelCenter);
	//bool hitPlane(const Ray& ray, const Vector3& pointOnPlane, const Vector3& planeNormal);

	std::vector<Vector3> m_sphereList;
	std::vector<RGB> m_sphereColours;
};

