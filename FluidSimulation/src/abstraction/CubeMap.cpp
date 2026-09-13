#include "CubeMap.h"
#include "../vendor/stb_image/stb_image.h"
#include "../debug/Log.h"

CubeMap::CubeMap(const std::string& path) : CubeMap(std::array<std::string, 6>{ path, path, path, path, path, path })
{

}

CubeMap::CubeMap(const std::array<std::string, 6>&paths) : m_RendererId(0), m_FilePaths(paths), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(0);

	glGenTextures(1, &m_RendererId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);

	for (unsigned int i = 0; i < 6; i++) {
		m_LocalBuffer = stbi_load(paths[i].c_str(), &m_Width, &m_Height, &m_BPP, 4);
		if (m_LocalBuffer) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
			stbi_image_free(m_LocalBuffer);
		}
		else {
			SPHWARN("Cubemap tex failed to load at path: " + paths[i]);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	stbi_set_flip_vertically_on_load(1);
}

CubeMap CubeMap::CreateEmpty(int width, int height, GLenum internalFormat)
{
	CubeMap cubeMap;
	cubeMap.m_Width = width;
	cubeMap.m_Height = height;

	glGenTextures(1, &cubeMap.m_RendererId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.m_RendererId);

	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, cubeMap.m_Width, cubeMap.m_Height, 0, internalFormat, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubeMap;
}

void CubeMap::Bind(unsigned int slot) const
{
	m_LastBoundSlot = slot;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);
}

void CubeMap::Unbind(unsigned int slot) const
{
	glBindTextureUnit(slot, 0);
}

CubeMap::CubeMap(CubeMap && other) noexcept
{
	m_RendererId = other.m_RendererId;
	m_FilePaths = other.m_FilePaths;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_BPP = other.m_BPP;

	other.m_RendererId = 0;
}

CubeMap& CubeMap::operator=(CubeMap && other) noexcept
{
	if (this == &other) return *this;

	if (m_RendererId != 0) glDeleteTextures(1, &m_RendererId);

	m_RendererId = other.m_RendererId;
	m_FilePaths = other.m_FilePaths;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_BPP = other.m_BPP;

	other.m_RendererId = 0;

	return *this;
}

CubeMap::~CubeMap()
{
	if (m_RendererId != 0) glDeleteTextures(1, &m_RendererId);
}
