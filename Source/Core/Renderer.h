#pragma once

#include "Camera.h"

class Framebuffer;
class Pathtracer;
class PerformanceCounter;
class SceneManager;
class TraversalDataManager;

// --------------------------------------------------------------------------------
class Renderer
{
public:

	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Camera* GetCamera();
	void Update(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc);

private:

	Camera m_camera;

	bool m_isFirstFrame;

	TraversalDataManager* m_traversalDataManager;
	SceneManager* m_sceneManager;

	Pathtracer* m_pathtracer;
};

