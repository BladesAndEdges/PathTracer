#pragma once

#include <vector>

#include "Vector3.h"

class Camera;
class Framebuffer;
struct HitResult;
class Ray;
class SceneManager;
class TraversalDataManager;

// --------------------------------------------------------------------------------
class Pathtracer
{
public: 

	Pathtracer();

	void CreateImage();

	void GenerateViewspaceDirections(const uint32_t imageWidth, const uint32_t imageHeight);

	void RenderSurfaceColour(Camera* camera, Framebuffer* framebuffer, 
		const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, uint8_t* out_pixels);

	// Make tracing functions private later
	template<bool T_acceptAnyHit>
	HitResult ScalarTraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, 
		Ray& ray);

	template<bool T_acceptAnyHit>
	HitResult SSETraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, 
		Ray& ray);

	template<bool T_acceptAnyHit>
	HitResult BVH2TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Ray& ray);

	template<bool T_acceptAnyHit>
	HitResult BVH4TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Ray& ray);

	const std::vector<Vector3>& GetViewSpaceDirections() const;
	float GetPixelWidth() const;
	float GetPixelHeight() const;

private:

	std::vector<Vector3> m_viewspaceDirections;

	float m_pixelWidth;
	float m_pixelHeight;

};

