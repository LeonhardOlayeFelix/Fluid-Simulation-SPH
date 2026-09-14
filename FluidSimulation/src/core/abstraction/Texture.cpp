#include "Texture.h"
#include "../../vendor/stb_image/stb_image.h"

Texture::Texture(const std::string path, GLenum internalFormat) : m_RendererId(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(1);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);
    glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
    glGenerateTextureMipmap(m_RendererId);

    if (m_LocalBuffer) stbi_image_free(m_LocalBuffer);
}

void Texture::Bind(unsigned int slot) const
{
    m_LastBoundSlot = slot;
    glBindTextureUnit(slot, m_RendererId);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture Texture::CreateEmpty(int width, int height, GLenum internalFormat, int samples)
{
    Texture tex;
    tex.m_Width = width;
    tex.m_Height = height;
    tex.m_LocalBuffer = nullptr;

    GLenum format, type;
    switch (internalFormat)
    {
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32F:
        format = GL_DEPTH_COMPONENT;
        type = GL_FLOAT;
        break;
    case GL_DEPTH24_STENCIL8:
        format = GL_DEPTH_STENCIL;
        type = GL_UNSIGNED_INT_24_8;
        break;
    default:
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    }

    if (samples > 1)
    {
        glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &tex.m_RendererId);
        glTextureStorage2DMultisample(tex.m_RendererId, samples, internalFormat, width, height, GL_TRUE);
    }
    else {
        glCreateTextures(GL_TEXTURE_2D, 1, &tex.m_RendererId);
        glTextureStorage2D(tex.m_RendererId, 1, internalFormat, width, height);
        glTextureParameteri(tex.m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(tex.m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return tex;
}

Texture::~Texture()
{
    if (m_RendererId != 0) glDeleteTextures(1, &m_RendererId);
}

Texture::Texture(Texture && other) noexcept
{
    m_RendererId = other.m_RendererId;
    m_FilePath = other.m_FilePath;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_BPP = other.m_BPP;

    other.m_RendererId = 0;
}

Texture& Texture::operator=(Texture && other) noexcept
{
    if (this == &other) return *this;

    if (m_RendererId != 0) glDeleteTextures(1, &m_RendererId);

    m_RendererId = other.m_RendererId;
    m_FilePath = other.m_FilePath;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_BPP = other.m_BPP;

    other.m_RendererId = 0;

    return *this;
}
