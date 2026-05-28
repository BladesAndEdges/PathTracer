#pragma once

#include <vector>

#include "Vector3.h"

// --------------------------------------------------------------------------------
class Pathtracer
{
public: 

	Pathtracer();

	void CreateImage();

	void GenerateViewspaceDirections(const uint32_t imageWidth, const uint32_t imageHeight);

	const std::vector<Vector3>& GetViewSpaceDirections() const;
	float GetPixelWidth() const;
	float GetPixelHeight() const;

private:


	std::vector<Vector3> m_viewspaceDirections;

	float m_pixelWidth;
	float m_pixelHeight;

};

