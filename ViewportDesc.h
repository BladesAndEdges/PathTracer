#pragma once

struct ViewportDesc
{
	float m_aspectRatio;
	float m_width;
	float m_height;
	float m_halfFov;
	float m_distanceToPlane;
	float m_texelWidth;
	float m_texelHeight;

	// Corners of the plane
	Vector3 m_topLeftTexel;
	Vector3 m_bottomLeftTexel;
	Vector3 m_topRightTexel;
	Vector3 m_bottomRightTexel;
};
