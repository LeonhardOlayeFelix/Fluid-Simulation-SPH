#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"
#include "VertexArray.h"

class Renderer
{
public:
	Renderer();
	void DrawElements(const VertexArray& vao, const ShaderProgram& shader) const;
	void DrawElementsInstanced(const VertexArray& vao, const ShaderProgram& shader, int nInstances) const;
	void DrawArray(const VertexArray& vao, const ShaderProgram& shader, GLenum mode = GL_TRIANGLES) const;
	void DrawArrayInstanced(const VertexArray& vao, const ShaderProgram& shader, int nInstances) const;
	void Clear() const;

};

