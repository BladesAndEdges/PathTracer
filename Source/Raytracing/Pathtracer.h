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

	void GenerateViewspaceDirections(const uint32_t imageWidth, const uint32_t imageHeight);

	void RenderPathtrace(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderInShadow(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderDepth(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderNormals(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderPrimitiveIds(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderMaterialIds(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

	void RenderTextureCoordinates(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, 
		Framebuffer* out_framebuffer);

	void RenderSurfaceColour(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
		Framebuffer* out_framebuffer);

private:

	Vector3 Pathtrace(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, const uint32_t depth,
		Ray& ray);

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

	std::vector<Vector3> m_viewspaceDirections;

	float m_pixelWidth;
	float m_pixelHeight;

	Vector3 m_lightDirection;
};

