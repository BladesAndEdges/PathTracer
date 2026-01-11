#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --------------------------------------------------------------------------------
TextureManager::TextureManager()
{
}

// --------------------------------------------------------------------------------
uint32_t TextureManager::Load(const char* textureName)
{
	std::string texture(textureName);
	uint32_t index = UINT32_MAX;

	bool found = false;
	for (uint32_t name = 0u; name < m_textureNames.size(); name++)
	{
		if (m_textureNames[name] == texture)
		{
			found = true;
			index = name;
		}
	}

	if (!found)
	{
		int x = INT_MAX;
		int y = INT_MAX;
		int channels = INT_MAX;
		const std::string path = std::string("Scenes/Sponza/") + texture;
		const uint8_t* data = stbi_load(path.data(), &x, &y, &channels, 0);

		if (!data)
		{
			__debugbreak();
		}
		else
		{
			m_textureNames.push_back(textureName);
			m_data.push_back(data);
			index = (uint32_t)m_textureNames.size() - 1u;
		}
	}

	return index;
}