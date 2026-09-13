#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../debug/Log.h"

struct VertexAttribute {
	unsigned int type;
	unsigned int count;
	unsigned char normalised;

	static unsigned int GetSizeOfType(unsigned int type) {
		switch (type) {
		case GL_FLOAT:        return sizeof(GLfloat);
		case GL_UNSIGNED_INT: return sizeof(GLuint);
		case GL_UNSIGNED_BYTE: return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexAttribute> m_Attributes{};
	unsigned int m_Stride{ 0 };
public:
	VertexBufferLayout() = default;
	VertexBufferLayout(std::initializer_list<unsigned int> nums);

	template<typename T>
	void push(unsigned int count) {
		static_assert(false);
	}

	template<>
	void push<float>(unsigned int count) {
		m_Attributes.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void push<unsigned int>(unsigned int count) {
		m_Attributes.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_UNSIGNED_INT);
	}
	template<>
	void push<unsigned char>(unsigned int count) {
		m_Attributes.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_UNSIGNED_BYTE);
	}

	inline const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }
	inline unsigned int GetStride() const { return m_Stride; }

};

