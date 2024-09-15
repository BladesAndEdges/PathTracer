#pragma once

#include<cstdint>

class Framebuffer;
class Vector3;

class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void UpdateFramebufferContents(Framebuffer* framebuffer, uint32_t frame, bool& changeColour);

private:

	bool hitSphere(const Vector3& rayOrigin, const Vector3& rayDirection, const Vector3& sphereCenter);
};

