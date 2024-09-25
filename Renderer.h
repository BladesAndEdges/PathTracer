#pragma once

#include<cstdint>
#include <vector>
#include "Vector3.h"

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

	bool hitSphere(const Vector3& texelCenter);
	bool hitPlane(const Ray& ray, const Vector3& pointOnPlane, const Vector3& planeNormal);

	std::vector<Vector3> m_sphereList;
};

