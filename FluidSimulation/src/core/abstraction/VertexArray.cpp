#include "VertexArray.h"

VertexArray::VertexArray() : m_VBOSize(0), m_Stride(0), m_AttributeIndex(0), m_IndexBuffer(nullptr)
{
	glCreateVertexArrays(1, &m_RendererId);
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererId);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::RecordVBOLayout(const VertexBuffer& vbo, const VertexBufferLayout& vbl)
{
	Bind();
	vbo.Bind();
	m_VBOSize += vbo.GetSize();
	m_Stride += vbl.GetStride();

	const std::vector<VertexAttribute>& attributes = vbl.GetAttributes();

	unsigned int offset = 0;

	for (const VertexAttribute& attribute : attributes) {

		glEnableVertexAttribArray(m_AttributeIndex);

		glVertexAttribPointer(m_AttributeIndex, attribute.count, attribute.type, attribute.normalised, vbl.GetStride(), (const void*)offset);

		offset += attribute.count * VertexAttribute::GetSizeOfType(attribute.type);

		m_AttributeIndex++;
	}
}

void VertexArray::RecordVBOLayout(const VertexBuffer & vbo, std::initializer_list<unsigned int> floatCounts)
{
	RecordVBOLayout(vbo, VertexBufferLayout(floatCounts));
}

void VertexArray::RecordIndexBuffer(const IndexBuffer & ibo)
{
	Bind();
	ibo.Bind();
	m_IndexBuffer = &ibo;
}

void VertexArray::SetAttribDivisor(unsigned int index, unsigned int divisor)
{
	Bind();
	glVertexAttribDivisor(index, divisor);
}

VertexArray::~VertexArray()
{
	if (m_RendererId != 0) glDeleteVertexArrays(1, &m_RendererId);
}

VertexArray::VertexArray(VertexArray && other) noexcept
{
	m_RendererId = other.m_RendererId;
	m_IndexBuffer = other.m_IndexBuffer;
	m_VBOSize = other.m_VBOSize;
	m_Stride = other.m_Stride;
	m_AttributeIndex = other.m_AttributeIndex;

	other.m_RendererId = 0;
	other.m_IndexBuffer = nullptr;
	other.m_VBOSize = 0;
	other.m_Stride = 0;
	other.m_AttributeIndex = 0;
}

VertexArray& VertexArray::operator=(VertexArray && other) noexcept
{
	if (this == &other) return *this;

	if (m_RendererId != 0) glDeleteVertexArrays(1, &m_RendererId);

	m_RendererId = other.m_RendererId;
	m_IndexBuffer = other.m_IndexBuffer;
	m_VBOSize = other.m_VBOSize;
	m_Stride = other.m_Stride;
	m_AttributeIndex = other.m_AttributeIndex;

	other.m_RendererId = 0;
	other.m_IndexBuffer = nullptr;
	other.m_VBOSize = 0;
	other.m_Stride = 0;
	other.m_AttributeIndex = 0;

	return *this;
}
