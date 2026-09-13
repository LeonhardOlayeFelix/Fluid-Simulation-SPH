#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
{
	glCreateBuffers(1, &m_RendererId);
	glNamedBufferData(m_RendererId, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
}

void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	if (m_RendererId != 0) glDeleteBuffers(1, &m_RendererId);
}

IndexBuffer::IndexBuffer(IndexBuffer && other) noexcept
{
	m_RendererId = other.m_RendererId;
	m_Count = other.m_Count;

	other.m_RendererId = 0;
	other.m_Count = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer && other) noexcept
{
	if (this == &other) return *this;

	if (m_RendererId != 0) glDeleteBuffers(1, &m_RendererId);

	m_RendererId = other.m_RendererId;
	m_Count = other.m_Count;

	other.m_RendererId = 0;
	other.m_Count = 0;

	return *this;
}
