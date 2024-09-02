#pragma once

#include<cstdint>

class Framebuffer;

class Renderer
{

public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void UpdateFramebufferContents(Framebuffer* framebuffer, uint32_t frame, bool& changeColour);
};

