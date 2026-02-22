#include "MaterialManager.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

#include "TextureManager.h"
#include "Vector3.h"

const Vector3 debugColours[20u]
{
	Vector3(1.0f, 0.0f, 0.0f),
	Vector3(0.0f, 1.0f, 0.0f),
	Vector3(0.0f, 0.0f, 1.0f),
	Vector3(1.0f, 1.0f, 0.0f),
	Vector3(1.0f, 0.0f, 1.0f),
	Vector3(0.0f, 1.0f, 1.0f),

	Vector3(0.5f, 0.0f, 0.0f),
	Vector3(0.0f, 0.5f, 0.0f),
	Vector3(0.0f, 0.0f, 0.5f),
	Vector3(0.5f, 0.5f, 0.0f),
	Vector3(0.5f, 0.0f, 0.5f),
	Vector3(0.0f, 0.5f, 0.5f),

	Vector3(1.0f, 0.3f, 0.0f),
	Vector3(0.0f, 1.0f, 0.3f),
	Vector3(0.3f, 0.0f, 1.0f),
	Vector3(1.0f, 0.3f, 0.3f),
	Vector3(1.0f, 0.3f, 1.0f),
	Vector3(0.3f, 1.0f, 1.0f),

	Vector3(1.0f, 0.3f, 0.5f),
	Vector3(1.0f, 1.0f, 1.0f)
};

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
uint32_t MaterialManager::GetMaterialIndexByName(const std::string& name) const
{
	for (uint32_t index = 0u; index < m_materialNames.size(); index++)
	{
		if (name == m_materialNames[index])
		{
			return index;
		}
	}

	return UINT32_MAX;
}

// --------------------------------------------------------------------------------
Vector3 MaterialManager::GetDebugMaterialColour(const uint32_t index) const
{
	return debugColours[index];
}

// --------------------------------------------------------------------------------
Vector3 MaterialManager::BasicSample(const uint32_t materialId, const float u, const float v) const
{
	const Material& material = m_materials[materialId];
	return m_textureManager->BasicSample(material, u, v);
}

// --------------------------------------------------------------------------------
void MaterialManager::Load(const char* mtlFile)
{
	const std::string path = "./Scenes/CrytekSponza/" + std::string(mtlFile);

	std::ifstream ifs(path);
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
			
			const size_t forwardSlashCheck = diffuseTexture.find_last_of('/');
			const uint32_t forwardSlashPos = (uint32_t)((forwardSlashCheck != std::string::npos) ? forwardSlashCheck : 0);

			const size_t backwardSlashCheck = diffuseTexture.find_last_of('\\');
			const uint32_t backwardSlashPos = (uint32_t)((backwardSlashCheck != std::string::npos) ? backwardSlashCheck : 0);

			const uint32_t nameStartPos = (forwardSlashPos > backwardSlashPos) ? forwardSlashPos + 1u : backwardSlashPos + 1u;
			const std::string diffuseTextureSubstr = diffuseTexture.substr(nameStartPos, std::string::npos);

			material.diffuseIndex = m_textureManager->Load(diffuseTextureSubstr.data());
		}
	}
}
