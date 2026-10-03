#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"
#include "VertexArray.h"

class RenderCommand
{
private:
	RenderCommand();

public:
	static void DrawElements(const VertexArray& vao, const ShaderProgram& shader);
	static void DrawElementsInstanced(const VertexArray& vao, const ShaderProgram& shader, int nInstances);
	static void DrawArray(const VertexArray& vao, const ShaderProgram& shader, GLenum mode = GL_TRIANGLES);
	static void DrawArrayInstanced(const VertexArray& vao, const ShaderProgram& shader, int nInstances);
	static void Clear();

};

