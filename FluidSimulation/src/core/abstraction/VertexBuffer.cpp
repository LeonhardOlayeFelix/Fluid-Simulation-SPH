#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const void* data, size_t size) : m_Size(size)
{
    glCreateBuffers(1, &m_RendererId);
    glNamedBufferData(m_RendererId, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    if (m_RendererId != 0) glDeleteBuffers(1, &m_RendererId);
}

VertexBuffer::VertexBuffer(VertexBuffer && other) noexcept
{
    m_RendererId = other.m_RendererId;
    m_Size = other.m_Size;

    other.m_RendererId = 0;
    other.m_Size = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer && other) noexcept
{
    if (this == &other) return *this;

    if (m_RendererId != 0) glDeleteBuffers(1, &m_RendererId);

    m_RendererId = other.m_RendererId;
    m_Size = other.m_Size;

    other.m_RendererId = 0;
    other.m_Size = 0;

    return *this;
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
