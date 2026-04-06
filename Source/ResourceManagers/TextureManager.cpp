#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "MaterialManager.h"
#include "Vector3.h"

// --------------------------------------------------------------------------------
TextureManager::TextureManager()
{
}

// --------------------------------------------------------------------------------
uint32_t TextureManager::Load(const char* textureName)
{
	std::string textureStr(textureName);
	uint32_t index = UINT32_MAX;

	bool found = false;
	for (uint32_t name = 0u; name < m_textureNames.size(); name++)
	{
		if (m_textureNames[name] == textureStr)
		{
			found = true;
			index = name;
		}
	}

	if (!found)
	{
		// Enforce bottom left to be the first texel
		stbi_set_flip_vertically_on_load(true);

		int x = INT_MAX;
		int y = INT_MAX;
		int channels = INT_MAX;
		const std::string path = std::string("Scenes/CrytekSponza/textures/") + textureStr;
		const uint8_t* data = stbi_load(path.data(), &x, &y, &channels, 3);

		if (!data)
		{
			__debugbreak();
		}
		else
		{
			m_textureNames.push_back(textureName);

			assert(sizeof(RGB) == 3u);

			Texture texture;
			texture.m_data = (const RGB*)data;
			texture.m_width = (uint32_t)x;
			texture.m_height = (uint32_t)y;
			m_textures.push_back(texture);

			index = (uint32_t)m_textureNames.size() - 1u;
		}
	}

	return index;
}

// --------------------------------------------------------------------------------
float SrgbToLinear(const float unormSrgb)
{
	float unormLinear;
	if (unormSrgb <= 0.04045f)
	{
		unormLinear = unormSrgb / 12.92f;
	}
	else
	{
		unormLinear = std::powf((unormSrgb + 0.055f) / 1.055f, 2.4f);
	}

	return unormLinear;
}

// --------------------------------------------------------------------------------
Vector3 TextureManager::BasicSample(const Material& material, const float u, const float v) const
{	
	const uint32_t diffuseTexture = material.diffuseIndex;
	const Texture& texture = m_textures[diffuseTexture];

	const float wrappedU = u - std::floor(u);
	const float wrappedV = v - std::floor(v);

	assert(wrappedU >= 0.0f);
	assert(wrappedU <= 1.0f);
	assert(wrappedV >= 0.0f);
	assert(wrappedV <= 1.0f);

	const uint32_t texelXStart = (uint32_t)(wrappedU * (float)(texture.m_width -1u));
	const uint32_t texelYStart = (uint32_t)(wrappedV * (float)(texture.m_height - 1u));

	assert(texelXStart >= 0u);
	assert(texelXStart < texture.m_width);
	assert(texelYStart >= 0u);
	assert(texelYStart < texture.m_height);

	const RGB start = texture.m_data[(texelYStart * texture.m_width) + texelXStart];

	const float unormRedSrgb = (float)start.red / 255.0f;
	const float unormGreenSrgb = (float)start.green / 255.0f;
	const float unormBlueSrgb = (float)start.blue / 255.0f;

	// Unorm Linear conversion
	const float unormRedLinear = SrgbToLinear(unormRedSrgb);
	const float unormGreenLinear = SrgbToLinear(unormGreenSrgb);
	const float unormBlueLinear = SrgbToLinear(unormBlueSrgb);

	// Bilineer comes after conversion if I ever do this.

	return Vector3(unormRedLinear, unormGreenLinear, unormBlueLinear);
}
