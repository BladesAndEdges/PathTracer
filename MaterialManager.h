#pragma once

#include <stdint.h>
#include <string>
#include <vector>

struct Material
{
	std::string path;
};

// --------------------------------------------------------------------------------
class MaterialManager
{

public:

	MaterialManager(const char* mtlFile);

	MaterialManager(const MaterialManager&) = delete;
	MaterialManager& operator=(const MaterialManager&) = delete;

private:

	void Load(const char* mtlFile);
	void ProcessMaterial(std::ifstream& ifs, Material& material);

	std::vector<Material> m_materials;
	std::vector<std::string> m_materialNames;
};

