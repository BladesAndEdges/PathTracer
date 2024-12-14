#pragma once

#include <string>
#include <vector>

#include "Face.h"
#include "Vector3.h"
#include "Vertex.h"

// --------------------------------------------------------------------------------
class ModelParser
{
public:

	ModelParser();

	void ParseFile(const char* objSourceFile, const float scaleFactor);

	Vector3 GetCenter() const;

	std::vector<float> GetPositionsX() const;
	std::vector<float> GetPositionsY() const;
	std::vector<float> GetPositionsZ() const;

	std::vector<Face> GetFaces() const;

private:

	void CreateFaces(const std::string& fileName, const float scaleFactor);

	void CalculateSceneCenter();

	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY, 
		std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ);
	void Triangulate(const std::vector<float>& faceX, const std::vector<float>& faceY, const std::vector<float>& faceZ);

	//----------------------------------------------
	void CreateFacesWithFaces(const std::string& fileName, const float scaleFactor);
	void ParseAttributesWithFaces(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	Vector3 CalculateSceneCenterWithFaces();
	void TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Face>& faces);

	//----------------------------------------------

	std::vector<float> m_positionsX;
	std::vector<float> m_positionsY;
	std::vector<float> m_positionsZ;

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;

	std::vector<Face> m_faces;

	Vector3 m_center;
};