#include "Framebuffer.h"

#include <cstring>
#include <glad/glad.h>

// --------------------------------------------------------------------------------
Framebuffer::Framebuffer(uint32_t width, uint32_t height, uint32_t channels) : m_width(width), m_height(height), m_channels(channels)
{
	m_data = new GLubyte[m_width * m_height * m_channels];

	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);

	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage2D(m_texture, 1, GL_RGB8, m_width, m_height);
}

// --------------------------------------------------------------------------------
void Framebuffer::use() const
{
	// If a new bind slot is used, make sure that you update it here
	glBindTextureUnit(0, m_texture);
}

// --------------------------------------------------------------------------------
void Framebuffer::update() const
{
	glTextureSubImage2D(m_texture, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_data);
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
uint32_t Framebuffer::getWidth() const
{
	return m_width;
}

// --------------------------------------------------------------------------------
uint32_t Framebuffer::getHeight() const
{
	return m_height;
}

// --------------------------------------------------------------------------------
uint32_t Framebuffer::getChannels() const
{
	return m_channels;
}

// --------------------------------------------------------------------------------
uint8_t* Framebuffer::getData()
{
	return m_data;
}
