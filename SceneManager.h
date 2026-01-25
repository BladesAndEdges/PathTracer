#pragma once

#include <string>
#include <vector>

#include "Vector3.h"

class MaterialManager;
struct Triangle;
class Vector2;
struct Vertex;

// --------------------------------------------------------------------------------
class SceneManager
{
public:

	SceneManager(const std::string& objFile, const std::string& mtlFile);

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	const std::vector<Triangle>& GetTriangles() const;
	Vector3 GetInitialCameraPosition() const;

private:

	void Load(const std::string& objFile);

	void CreateVertices(const std::string& line, const std::vector<Vector3>& positions,
		const std::vector<Vector2>& textureCoordinates, std::vector<Vertex>& out_vertices) const;
	void Triangulate(const std::vector<Vertex>& vertices, std::vector<Triangle>& out_triangles) const;

	MaterialManager* m_materialManager;
	std::vector<Triangle> m_triangles;
	std::vector<uint32_t> m_triangleMaterials;

	Vector3 m_initialCameraPos;
};

