#pragma once

#include <stdint.h>
#include <string>
#include <vector>

class TextureManager;
class Vector3;

struct Material
{
	uint32_t diffuseIndex;
};

// --------------------------------------------------------------------------------
class MaterialManager
{

public:

	MaterialManager(const char* mtlFile);

	MaterialManager(const MaterialManager&) = delete;
	MaterialManager& operator=(const MaterialManager&) = delete;

	uint32_t GetMaterialIndexByName(const std::string& name) const;
	Vector3 GetDebugMaterialColour(const uint32_t index) const;

private:

	void Load(const char* mtlFile);
	void ProcessMaterial(std::ifstream& ifs, Material& material);

	std::vector<Material> m_materials;
	std::vector<std::string> m_materialNames;

	TextureManager* m_textureManager;
};

