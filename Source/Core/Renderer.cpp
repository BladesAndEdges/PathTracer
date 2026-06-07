#include "Renderer.h"

#include "Framebuffer.h"
#include "Pathtracer.h"
#include "PerformanceCounter.h"
#include "SceneManager.h"
#include "TraversalDataManager.h"

// --------------------------------------------------------------------------------
Renderer::Renderer()
{
	m_isFirstFrame = true;

	m_sceneManager = new SceneManager("sponza.obj", 0.01f, "sponza.mtl");

	m_traversalDataManager = new TraversalDataManager(m_sceneManager->GetTriangles(), m_sceneManager->GetPerTriangleMaterials());

	m_camera.SetCameraLocation(m_sceneManager->GetInitialCameraPosition());

	m_pathtracer = new Pathtracer();
}

// --------------------------------------------------------------------------------
Renderer::~Renderer()
{
	delete m_sceneManager;

	delete m_traversalDataManager;

	delete m_pathtracer;
}

// --------------------------------------------------------------------------------
Camera* Renderer::GetCamera()
{
	return &m_camera;
}

// --------------------------------------------------------------------------------
void Renderer::Update(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc)
{
	const SHORT zKeyState = GetAsyncKeyState(0x5A);
	const SHORT xKeyState = GetAsyncKeyState(0x58);
	const SHORT cKeyState = GetAsyncKeyState(0x43);
	const SHORT vKeyState = GetAsyncKeyState(0x56);
	const SHORT bKeyState = GetAsyncKeyState(0x42);
	const SHORT nKeyState = GetAsyncKeyState(0x4E);
	const SHORT mKeyState = GetAsyncKeyState(0x4D);

	if (hasResized || m_isFirstFrame)
	{
		m_pathtracer->GenerateViewspaceDirections(framebuffer->GetWidth(), framebuffer->GetHeight());
		m_isFirstFrame = false;
	}

	pc.BeginTiming();

	if (zKeyState > 0u)
	{
		m_pathtracer->RenderInShadow(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (xKeyState > 0u)
	{
		m_pathtracer->RenderDepth(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (cKeyState > 0u)
	{
		m_pathtracer->RenderNormals(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (vKeyState)
	{
		m_pathtracer->RenderPrimitiveIds(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (bKeyState)
	{
		m_pathtracer->RenderMaterialIds(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (nKeyState)
	{
		m_pathtracer->RenderTextureCoordinates(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else if (mKeyState)
	{
		m_pathtracer->RenderSurfaceColour(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}
	else
	{
		m_pathtracer->RenderPathtrace(&m_camera, m_traversalDataManager, m_sceneManager, framebuffer);
	}

	pc.EndTiming();

	char buffer[128];
	sprintf_s(buffer, "Total frame time: %f \n", pc.GetMilliseconds());
	OutputDebugStringA(buffer);
}