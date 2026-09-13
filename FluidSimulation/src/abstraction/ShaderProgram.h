#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
	std::string GeometrySource;
};

class ShaderProgram
{
private:
	std::string m_FilePath{};
	unsigned int m_RendererId{};
	std::unordered_map<std::string, int> m_UniformLocationCache{};

public:
	ShaderProgram() = default;
	ShaderProgram(const std::string& filename);

	void Bind() const;
	void Unbind() const;

	void SetUniform4f(const std::string& name, const glm::vec4& vec);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform3f(const std::string& name, const glm::vec3& vec);
	void SetUniform2f(const std::string& name, const glm::vec2& vec);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform2f(const std::string& name, float v0, float v1);
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	void SetUniformMat3f(const std::string& name, const glm::mat3& matrix);
	void SetUniform1fv(const std::string& name, int count, const float* values);

	~ShaderProgram();
	ShaderProgram(const ShaderProgram& other) = delete;
	ShaderProgram& operator=(const ShaderProgram& other) = delete;
	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;

	inline int GetID() { return m_RendererId; }

private:
	ShaderProgramSource ParseShaderProgram(const std::string& filepath);
	unsigned int CreateShaderProgram(const ShaderProgramSource&);
	unsigned int AttachShaderToProgram(unsigned int program, const std::string& shaderSource, GLenum type);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	int GetUniformLocation(const std::string& name);
	int GetUniformBlockIndex(const std::string& name);
};

