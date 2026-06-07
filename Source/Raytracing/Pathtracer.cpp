#include "Pathtracer.h"

#include "BaseTypes4.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "HitResult.h"
#include "Intersections.h"
#include "Ray.h"
#include "SceneManager.h"
#include "Traversals.h"
#include "TraversalDataManager.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"

# define NOMINMAX
# define M_PI 3.14159265358979323846
# define TMIN 1e-5f

//#define RENDER_SCALAR
//#define RENDER_SSE
//#define RENDER_BVH2
#define RENDER_BVH4

const Vector3 primitiveDebugColours[5u] = { Vector3(0.94f, 0.34f, 0.30f), Vector3(0.30f, 0.94f, 0.70f), Vector3(0.51f, 0.70f, 0.96f),
	Vector3(0.96f, 0.91f, 0.51f), Vector3(0.96f, 0.61f, 0.91f) };

// --------------------------------------------------------------------------------
Pathtracer::Pathtracer() : m_pixelWidth(0.0f), m_pixelHeight(0.0f)
{
	m_lightDirection = Normalize(Vector3(0.9f, 1.0f, 0.4f));
}

// --------------------------------------------------------------------------------
void Pathtracer::GenerateViewspaceDirections(const uint32_t framebufferWidth, const uint32_t framebufferHeight)
{
	m_viewspaceDirections.clear();

	const float aspectRatio = (float)framebufferWidth / (float)framebufferHeight;
	const float width = 2.0f;
	const float height = width / aspectRatio;
	const float halfFov = 45.0f * ((float)M_PI / 180.0f);
	const float distanceToPlane = (width / 2.0f) / tanf(halfFov);

	m_pixelWidth = width / framebufferWidth;
	m_pixelHeight = height / framebufferHeight;

	// Corners of the plane
	const Vector3 topLeft(-width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomLeft(-width / 2.0f, -height / 2.0f, -distanceToPlane);
	const Vector3 topRight(width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomRight(width / 2.0f, -height / 2.0f, -distanceToPlane);

	for (uint32_t row = 0u; row < framebufferHeight; row++)
	{
		const float ty = (row * m_pixelHeight) / height;
		const Vector3 r0 = (1.0f - ty) * topLeft + ty * bottomLeft;
		const Vector3 r1 = (1.0f - ty) * topRight + ty * bottomRight;

		for (uint32_t column = 0u; column < framebufferWidth; column++)
		{
			const float tx = (column * m_pixelWidth) / width;

			Vector3 texelCenter = (1.0f - tx) * r0 + tx * r1;
			const float xVal = texelCenter.X() + (m_pixelWidth / 2.0f);

			const float valDegreesToRad = 90.0f * ((float)M_PI / 180.0f);
			const float rotatedX = cosf(valDegreesToRad) * xVal + sinf(valDegreesToRad) * texelCenter.Z();
			const float rotatedZ = -sinf(valDegreesToRad) * xVal + cosf(valDegreesToRad) * texelCenter.Z();

			texelCenter.SetX(rotatedX);
			texelCenter.SetY(texelCenter.Y() - (m_pixelHeight / 2.0f));
			texelCenter.SetZ(rotatedZ);
			m_viewspaceDirections.push_back(texelCenter);
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderPathtrace(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* bytes = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			// Byte offsets
			const uint32_t texelByteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) + (column * out_framebuffer->GetNumChannels());
			assert(texelByteIndex < (out_framebuffer->GetWidth() * out_framebuffer->GetHeight() * out_framebuffer->GetNumChannels()));

			float red = 0.0f;
			float green = 0.0f;
			float blue = 0.0f;

			Vector3 radiance(0.0f, 0.0f, 0.0f);
			const uint32_t numSamples = 1u;
			const uint32_t depth = 2u;

			for (uint32_t sample = 0u; sample < numSamples; sample++)
			{
				Vector3 texelTopLeft;
				Vector3 texelBottomRight;

				texelTopLeft.SetX(m_viewspaceDirections[rayIndex].X() - m_pixelWidth / 2.0f);
				texelTopLeft.SetY(m_viewspaceDirections[rayIndex].Y() + m_pixelHeight / 2.0f);

				texelBottomRight.SetX(m_viewspaceDirections[rayIndex].X() + m_pixelWidth / 2.0f);
				texelBottomRight.SetY(m_viewspaceDirections[rayIndex].Y() - m_pixelHeight / 2.0f);

				const float randomX = RandomFloat(texelTopLeft.X(), texelBottomRight.X());
				const float randomY = RandomFloat(texelBottomRight.Y(), texelTopLeft.Y());

				Ray ray(camera->GetCameraLocation(), Vector3(randomX, randomY, m_viewspaceDirections[rayIndex].Z()));

				radiance = radiance + Pathtrace(traversalDataManager, sceneManager, depth, ray);
			}

			radiance = Vector3(radiance.X() / (float)numSamples, radiance.Y() / (float)numSamples, radiance.Z() / (float)numSamples);

			// Clamp prior to the conversion, assume SDR
			red = std::fmin(1.0f, radiance.X());
			green = std::fmin(1.0f, radiance.Y());
			blue = std::fmin(1.0f, radiance.Z());

			bytes[texelByteIndex] = uint8_t((red * 255.0f) + 0.5f);
			bytes[texelByteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			bytes[texelByteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			bytes[texelByteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderInShadow(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
			float red, green, blue;
			if (hr.m_t != INFINITY)
			{
				Ray shadowRay(hr.m_intersectionPoint, m_lightDirection);

#ifdef RENDER_SCALAR
						const HitResult shadowHr = ScalarTraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_SSE
						const HitResult shadowHr = SSETraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_BVH2
						const HitResult shadowHr = BVH2TraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_BVH4
						const HitResult shadowHr = BVH4TraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
						red = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
						green = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
						blue = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
			}
			else
			{
				red = 0.7f;
				green = 0.7f;
				blue = 0.7f;
			}

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderDepth(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
			const float maxDepth = 10.0f;

			const float red = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
			const float green = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
			const float blue = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderNormals(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
			const float red = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.X() + 0.5f : 0.0f;
			const float green = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Y() + 0.5f : 0.0f;
			const float blue = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Z() + 0.5f : 0.0f;

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderPrimitiveIds(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
			const float red = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].X() : 0.0f;
			const float green = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Y() : 0.0f;
			const float blue = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Z() : 0.0f;

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderMaterialIds(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
			const float red = (hr.m_materialId != UINT32_MAX) ? sceneManager->GetDebugMaterialColour(hr.m_materialId).X() : 0.0f;
			const float green = (hr.m_materialId != UINT32_MAX) ? sceneManager->GetDebugMaterialColour(hr.m_materialId).Y() : 0.0f;
			const float blue = (hr.m_materialId != UINT32_MAX) ? sceneManager->GetDebugMaterialColour(hr.m_materialId).Z() : 0.0f;

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderTextureCoordinates(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, 
	Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif

			// Possibly do some other comparison due to fp precision
			const bool uInRange = ((hr.m_texCoords.X() >= 0.0f) && (hr.m_texCoords.X() <= 1.0f));
			const bool vInRange = ((hr.m_texCoords.Y() >= 0.0f) && (hr.m_texCoords.Y() <= 1.0f));

			const float red = (uInRange && vInRange) ? hr.m_texCoords.X() : 1.0f;
			const float green = (uInRange && vInRange) ? hr.m_texCoords.Y() : 0.75f;
			const float blue = (uInRange && vInRange) ? 0.0f : 0.8f;

			pixels[byteIndex] = uint8_t((red * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
void Pathtracer::RenderSurfaceColour(const Camera* camera, const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager,
	Framebuffer* out_framebuffer)
{
	uint8_t* pixels = out_framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < out_framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < out_framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * out_framebuffer->GetWidth() + column);

			const uint32_t byteIndex = (row * out_framebuffer->GetWidth() * out_framebuffer->GetNumChannels()) +
				(column * out_framebuffer->GetNumChannels());

			Ray ray(camera->GetCameraLocation(), m_viewspaceDirections[rayIndex]);

			// Trace based on selected method
#ifdef RENDER_SCALAR
			const HitResult hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_SSE
			const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
			const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
			const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif

			pixels[byteIndex] = uint8_t((hr.m_colour.X() * 255.0f) + 0.5f);
			pixels[byteIndex + 1u] = uint8_t((hr.m_colour.Y() * 255.0f) + 0.5f);
			pixels[byteIndex + 2u] = uint8_t((hr.m_colour.Z() * 255.0f) + 0.5f);
			pixels[byteIndex + 3u] = 255u;
		}
	}
}

// --------------------------------------------------------------------------------
Vector3 Pathtracer::Pathtrace(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, const uint32_t depth,
	Ray& ray)
{
	if (depth <= 0u)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 radiance(0.0f, 0.0f, 0.0f);

#ifdef RENDER_SCALAR
	const HitResult  hr = ScalarTraceRay<false>(traversalDataManager, sceneManager, ray);
#endif 
#ifdef RENDER_SSE
	const HitResult hr = SSETraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH2
	const HitResult hr = BVH2TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif
#ifdef RENDER_BVH4
	const HitResult hr = BVH4TraceRay<false>(traversalDataManager, sceneManager, ray);
#endif

	if (hr.m_t != INFINITY)
	{
		// Indirect lighting
		{
			// Calculate the random direction of the outward ray
			Ray rayOnHemisphere(hr.m_intersectionPoint, Vector3::RandomVector3OnHemisphere(hr.m_normal));

			// RENDERING EQUATION

			// We need the Li
			const Vector3 Li = Pathtrace(traversalDataManager, sceneManager, depth - 1u, rayOnHemisphere);

			// Elongation/cosine term, the falloff (Geometric term)
			// We use the ray direction, instead of -ray.Direction() so that the Dot product produces a positive value
			const float cosineTerm = std::fmin(std::fmax(Dot(rayOnHemisphere.Direction(), hr.m_normal), 0.0f), 1.0f);

			// BRDF, in our case just use Lambert which is P/PI, P being the colour of the material, a vector3 [0,1] for each wavelength
			const Vector3 brdf = (1.0f / (float)M_PI) * hr.m_colour;

			radiance = cosineTerm * brdf * Li;

			// Divide everything by the probability distribution function, for our case just 1/ 2 * Pi
			const float pdf = 1.0f / (2.0f * (float)M_PI);
			radiance = Vector3(radiance.X() / pdf, radiance.Y() / pdf, radiance.Z() / pdf);
		}

		//Direct Lighting
		{
			const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, hr.m_normal), 0.0f), 1.0f);

			Ray shadowRay(hr.m_intersectionPoint, m_lightDirection);

#ifdef RENDER_SCALAR
			const HitResult shadowHr = ScalarTraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_SSE
			const HitResult shadowHr = SSETraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_BVH2
			const HitResult shadowHr = BVH2TraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif
#ifdef RENDER_BVH4
			const HitResult shadowHr = BVH4TraceRay<true>(traversalDataManager, sceneManager, shadowRay);
#endif

			if (shadowHr.m_t == INFINITY)
			{
				Vector3 directRadiance;
				directRadiance.SetX(clampValue * hr.m_colour.X());
				directRadiance.SetY(clampValue * hr.m_colour.Y());
				directRadiance.SetZ(clampValue * hr.m_colour.Z());

				radiance = radiance + 2.0 * directRadiance;
			}
		}
	}
	else
	{
		const float skyIntensity = 1.5f;
		const Vector3 skyColour = skyIntensity * Vector3(0.98f, 0.68f, 0.37f);

		radiance.SetX(skyColour.X());
		radiance.SetY(skyColour.Y());
		radiance.SetZ(skyColour.Z());
	}

	return radiance;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::ScalarTraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	const std::vector<TraversalTriangle>& traversalTriangles = traversalDataManager->GetTraversalTriangles();
	const TraversalTriangle* const beginTriangle = &traversalTriangles[0u];
	const TraversalTriangle* const endTriangle = beginTriangle + traversalTriangles.size();

	const std::vector<uint32_t>& materialIndices = traversalDataManager->GetMaterialIndices();
	const std::vector<TriangleTexCoord>& triangleTexCoords = traversalDataManager->GetTriangleTexCoords();

	uint32_t triangleIndex = 0u;
	for (const TraversalTriangle* triangle = beginTriangle; triangle != endTriangle; triangle++)
	{
		HitTriangle(ray, *triangle, triangleIndex, TMIN, primitiveId, tMax, tu, tv, hasHit);
		
		if (T_acceptAnyHit)
		{
			if (hasHit)
			{
				break;
			}
		}

		triangleIndex++;
	}

	if (hasHit)
	{
		hitResult.m_t = tMax;

		if (!T_acceptAnyHit)
		{
			hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

			const TriangleTexCoord& texCoord = triangleTexCoords[primitiveId];
			hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoord.m_v0uv[0u] + tu * texCoord.m_v1uv[0u] + tv * texCoord.m_v2uv[0u]);
			hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoord.m_v0uv[1u] + tu * texCoord.m_v1uv[1u] + tv * texCoord.m_v2uv[1u]);

			const Vector3 edge1 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge1[0u], traversalTriangles[primitiveId].m_edge1[1u], traversalTriangles[primitiveId].m_edge1[2u]));
			const Vector3 edge2 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge2[0u], traversalTriangles[primitiveId].m_edge2[1u], traversalTriangles[primitiveId].m_edge2[2u]));
			const Vector3 normal = Normalize(Cross(edge1, edge2));
			hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

			hitResult.m_primitiveId = primitiveId;

			hitResult.m_materialId = materialIndices[primitiveId];

			hitResult.m_colour = sceneManager->BasicSample(materialIndices[primitiveId], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
		}
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::SSETraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;

	// Constants
	const __m128 epsilon = _mm_set1_ps(1e-8f);
	const __m128 zeros = _mm_set1_ps(0.0f);
	const __m128 ones = _mm_set1_ps(1.0f);

	// Ray data
	const __m128 rayOriginX = _mm_set1_ps(ray.Origin().X());
	const __m128 rayOriginY = _mm_set1_ps(ray.Origin().Y());
	const __m128 rayOriginZ = _mm_set1_ps(ray.Origin().Z());

	const __m128 rayDirectionX = _mm_set1_ps(ray.Direction().X());
	const __m128 rayDirectionY = _mm_set1_ps(ray.Direction().Y());
	const __m128 rayDirectionZ = _mm_set1_ps(ray.Direction().Z());

	// tMin
	const __m128 tMinimum = _mm_set1_ps(TMIN);

	// Loop outputs
	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);

	const std::vector<TraversalTriangle4>& traversalTriangle4s = traversalDataManager->GetSSETraversalTriangle4s();
	const TraversalTriangle4* const beginTriangle4 = &traversalTriangle4s[0u];
	const TraversalTriangle4* const endTriangle4 = beginTriangle4 + traversalTriangle4s.size();

	int currentIndex = 0u;
	int moveMask = 0u;
	for (const TraversalTriangle4* triangle4 = beginTriangle4; triangle4 != endTriangle4; triangle4++)
	{
		HitTriangle4(ray, *triangle4, currentIndex, TMIN, outTri4Indices, outTMax, outU, outV, moveMask);

		if (T_acceptAnyHit)
		{
			if (moveMask)
			{
				break;
			}
		}

		currentIndex++;
	}

	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));

	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));

	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));

	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));

	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);

	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];

		hitResult.m_t = tMax;

		if (!T_acceptAnyHit)
		{
			float us[4u];
			_mm_storeu_ps(us, outU);
			float u = us[subIndex];

			float vs[4u];
			_mm_storeu_ps(vs, outV);
			float v = vs[subIndex];

			hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

			const std::vector<TriangleTexCoord4>& texCoord4s = traversalDataManager->GetSSETriangleTexCoord4s();
			hitResult.m_texCoords.SetX((1.0f - u - v) * texCoord4s[tri4Index].m_v0U[subIndex] + u * texCoord4s[tri4Index].m_v1U[subIndex] + v * texCoord4s[tri4Index].m_v2U[subIndex]);
			hitResult.m_texCoords.SetY((1.0f - u - v) * texCoord4s[tri4Index].m_v0V[subIndex] + u * texCoord4s[tri4Index].m_v1V[subIndex] + v * texCoord4s[tri4Index].m_v2V[subIndex]);

			const Vector3 edge1 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge1X[subIndex],
				traversalTriangle4s[tri4Index].m_edge1Y[subIndex],
				traversalTriangle4s[tri4Index].m_edge1Z[subIndex]));

			const Vector3 edge2 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge2X[subIndex],
				traversalTriangle4s[tri4Index].m_edge2Y[subIndex],
				traversalTriangle4s[tri4Index].m_edge2Z[subIndex]));

			const Vector3 normal = Normalize(Cross(edge1, edge2));
			hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

			hitResult.m_primitiveId = (tri4Index * 4u) + subIndex;

			const std::vector<MaterialIndex4>& materialIndex4 = traversalDataManager->GetSSEMaterialIndex4s();
			hitResult.m_materialId = materialIndex4[tri4Index].m_index[subIndex];

			hitResult.m_colour = sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
		}
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::BVH2TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	BVH2Traversal<T_acceptAnyHit>(traversalDataManager, 0u, ray, TMIN, primitiveId, tMax, tu, tv, hasHit);

	if (hasHit)
	{
		hitResult.m_t = tMax;

		if (!T_acceptAnyHit)
		{
			hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

			const TriangleTexCoord& texCoord = traversalDataManager->GetBVH2TriangleTexCoord(primitiveId);
			hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoord.m_v0uv[0u] + tu * texCoord.m_v1uv[0u] + tv * texCoord.m_v2uv[0u]);
			hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoord.m_v0uv[1u] + tu * texCoord.m_v1uv[1u] + tv * texCoord.m_v2uv[1u]);

			const TraversalTriangle& traversalTriangle = traversalDataManager->GetBVH2TraversalTriangle(primitiveId);
			const Vector3 edge1 = Normalize(Vector3(traversalTriangle.m_edge1[0u], traversalTriangle.m_edge1[1u], traversalTriangle.m_edge1[2u]));
			const Vector3 edge2 = Normalize(Vector3(traversalTriangle.m_edge2[0u], traversalTriangle.m_edge2[1u], traversalTriangle.m_edge2[2u]));
			const Vector3 normal = Normalize(Cross(edge1, edge2));
			hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

			hitResult.m_primitiveId = primitiveId;

			hitResult.m_materialId = traversalDataManager->GetBVH2MaterialIndex(primitiveId);

			hitResult.m_colour = sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
		}
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Pathtracer::BVH4TraceRay(const TraversalDataManager* traversalDataManager, const SceneManager* sceneManager, Ray& ray)
{
	HitResult hitResult;

	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);
	int moveMask = 0;
	
	BVH4Traversal<T_acceptAnyHit>(traversalDataManager, 0u, ray, TMIN, outTri4Indices, outTMax, outU, outV, moveMask);
	
	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));
	
	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));
	
	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));
	
	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));
	
	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);
	
	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];

		hitResult.m_t = tMax;

		if (!T_acceptAnyHit)
		{
			float us[4u];
			_mm_storeu_ps(us, outU);
			float u = us[subIndex];

			float vs[4u];
			_mm_storeu_ps(vs, outV);
			float v = vs[subIndex];

			hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

			const TriangleTexCoord4& triangleTexCoord4 = traversalDataManager->GetBVH4TriangleTexCoord4(tri4Index);
			hitResult.m_texCoords.SetX((1.0f - u - v) * triangleTexCoord4.m_v0U[subIndex] + u * triangleTexCoord4.m_v1U[subIndex] + v * triangleTexCoord4.m_v2U[subIndex]);
			hitResult.m_texCoords.SetY((1.0f - u - v) * triangleTexCoord4.m_v0V[subIndex] + u * triangleTexCoord4.m_v1V[subIndex] + v * triangleTexCoord4.m_v2V[subIndex]);

			const TraversalTriangle4& traversalTriangle4 = traversalDataManager->GetBVH4TraversalTriangle4(tri4Index);
			const Vector3 edge1 = Normalize(Vector3(traversalTriangle4.m_edge1X[subIndex], traversalTriangle4.m_edge1Y[subIndex], traversalTriangle4.m_edge1Z[subIndex]));
			const Vector3 edge2 = Normalize(Vector3(traversalTriangle4.m_edge2X[subIndex], traversalTriangle4.m_edge2Y[subIndex], traversalTriangle4.m_edge2Z[subIndex]));
			const Vector3 normal = Normalize(Cross(edge1, edge2));
			hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

			const TriangleIndex4& triangleIndex4 = traversalDataManager->GetBVH4TriangleIndex4(tri4Index);
			hitResult.m_primitiveId = triangleIndex4.m_index[subIndex];

			const MaterialIndex4& materialIndex4 = traversalDataManager->GetBVH4MaterialIndex4(tri4Index);
			hitResult.m_materialId = materialIndex4.m_index[subIndex];

			hitResult.m_colour = sceneManager->BasicSample(materialIndex4.m_index[subIndex], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
		}
	}

	return hitResult;
}