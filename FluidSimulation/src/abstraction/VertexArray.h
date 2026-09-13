#pragma once
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
private:
	unsigned int m_RendererId{};
	unsigned int m_AttributeIndex{};
	unsigned int m_BindingIndex{};
	const IndexBuffer* m_IndexBuffer{};
	size_t m_VBOSize{};
	size_t m_Stride{};

public:
	VertexArray();

	void Bind() const;
	void Unbind() const;

	void RecordVBOLayout(const VertexBuffer& vbo, const VertexBufferLayout& vbl);
	void RecordVBOLayout(const VertexBuffer& vbo, std::initializer_list<unsigned int> floatCounts);
	void RecordIndexBuffer(const IndexBuffer& ibo);
	void SetAttribDivisor(unsigned int index, unsigned int divisor);

	~VertexArray();
	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;
	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& other) noexcept;

	inline unsigned int GetIndexBufferCount() const { return m_IndexBuffer->GetCount(); };
	inline size_t GetVertexBufferSize() const { return m_VBOSize; };
	inline size_t GetStride() const { return m_Stride; };
	inline unsigned int GetAttributeIndex() const { return m_AttributeIndex; }
};

