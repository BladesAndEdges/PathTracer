#include "MaterialManager.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

#include "TextureManager.h"

// --------------------------------------------------------------------------------
MaterialManager::MaterialManager(const char* mtlFile)
{
	std::filesystem::path filepath = mtlFile;
	if (filepath.extension() == ".mtl" || filepath.extension() == ".MTL")
	{
		m_textureManager = new TextureManager();
		Load(mtlFile);
		assert(m_materialNames.size() == m_materials.size());
	}
	else
	{
		assert(false);
	}
}

// --------------------------------------------------------------------------------
void MaterialManager::Load(const char* mtlFile)
{
	std::ifstream ifs(mtlFile);
	if (ifs.fail())
	{
		assert(false);
	}

	std::string word;
	while (ifs >> word)
	{
		if (ifs.fail())
		{
			break;
		}

		if (word == "newmtl")
		{
			ifs >> word;
			m_materialNames.push_back(word);
			std::getline(ifs, word);

			Material material;
			ProcessMaterial(ifs, material);
			
			m_materials.push_back(material);
		}

	}
}

// --------------------------------------------------------------------------------
void MaterialManager::ProcessMaterial(std::ifstream& ifs, Material& material)
{
	std::string line;
	std::string word;
	while (std::getline(ifs, line))
	{
		if (ifs.fail())
		{
			assert(false);
		}

		if (line == "")
		{
			break;
		}

		std::istringstream istream(line);
		istream >> word;
		if (word == "map_Kd")
		{
			std::string diffuseTexture;
			istream >> diffuseTexture;
			material.diffuseIndex = m_textureManager->Load(diffuseTexture.data()); // Would load otherwise when texture loading is made
		}
	}
}
