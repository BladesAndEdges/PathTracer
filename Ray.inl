// --------------------------------------------------------------------------------
inline Vector3 Ray::Origin() const
{
	return m_rayOrigin;
}

// --------------------------------------------------------------------------------
inline Vector3 Ray::Direction() const
{
	return m_normalizedRayDir;
}

// --------------------------------------------------------------------------------
inline Vector3 Ray::InverseDirection() const
{
	return m_inverseDirection;
}