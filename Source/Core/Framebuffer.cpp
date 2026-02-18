#include "Framebuffer.h"

#include <assert.h>
#include <cstring>
#include <glad/glad.h>

// --------------------------------------------------------------------------------
Framebuffer::Framebuffer(uint32_t width, uint32_t height, uint32_t channels) : m_width(width), m_height(height), m_numChannels(channels)
{
	m_data = new GLubyte[m_width * m_height * m_numChannels];

	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);

	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (m_numChannels == 3u)
	{
		glTextureStorage2D(m_texture, 1, GL_RGB8, m_width, m_height);
	}
	else if (m_numChannels == 4u)
	{
		glTextureStorage2D(m_texture, 1, GL_RGBA8, m_width, m_height);
	}
	else
	{
		assert(0u);
	}
}

// --------------------------------------------------------------------------------
void Framebuffer::Use() const
{
	// If a new bind slot is used, make sure that you update it here
	glBindTextureUnit(0, m_texture);
}

// --------------------------------------------------------------------------------
void Framebuffer::Update() const
{
	if (m_numChannels == 3u)
	{
		glTextureSubImage2D(m_texture, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_data);
	}
	else if (m_numChannels == 4u)
	{
		glTextureSubImage2D(m_texture, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
	}
	else
	{
		assert(0u);
	}
}

// --------------------------------------------------------------------------------
Framebuffer::~Framebuffer()
{
	if (m_texture != 0)
	{
		delete m_data;
		m_data = nullptr;

		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}
}

// --------------------------------------------------------------------------------
uint32_t Framebuffer::GetWidth() const
{
	return m_width;
}

// --------------------------------------------------------------------------------
uint32_t Framebuffer::GetHeight() const
{
	return m_height;
}

// --------------------------------------------------------------------------------
uint32_t Framebuffer::GetNumChannels() const
{
	return m_numChannels;
}

// --------------------------------------------------------------------------------
uint8_t* Framebuffer::GetDataPtr()
{
	return m_data;
}
