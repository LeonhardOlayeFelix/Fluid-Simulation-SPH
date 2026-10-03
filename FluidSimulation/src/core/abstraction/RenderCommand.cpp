#include "RenderCommand.h"

RenderCommand::RenderCommand()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.1, 0.1, 0.1, 1.0f);
}

void RenderCommand::DrawElements(const VertexArray & vao, const ShaderProgram & shader)
{
	shader.Bind();
	vao.Bind();
	glDrawElements(GL_TRIANGLES, vao.GetIndexBufferCount(), GL_UNSIGNED_INT, nullptr);
}

void RenderCommand::DrawElementsInstanced(const VertexArray & vao, const ShaderProgram & shader, int nInstances)
{
	shader.Bind();
	vao.Bind();
	glDrawElementsInstanced(GL_TRIANGLES, vao.GetIndexBufferCount(), GL_UNSIGNED_INT, nullptr, nInstances);
}

void RenderCommand::DrawArray(const VertexArray & vao, const ShaderProgram & shader, GLenum mode)
{
	shader.Bind();
	vao.Bind();
	glDrawArrays(mode, 0, vao.GetVertexBufferSize() / vao.GetStride());
}

void RenderCommand::DrawArrayInstanced(const VertexArray & vao, const ShaderProgram & shader, int nInstances)
{
	shader.Bind();
	vao.Bind();
	glDrawArraysInstanced(GL_TRIANGLES, 0, vao.GetVertexBufferSize() / vao.GetStride(), nInstances);
}

void RenderCommand::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


