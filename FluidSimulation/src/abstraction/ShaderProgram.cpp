#include "ShaderProgram.h"
#include <fstream>
#include <sstream>
#include "../debug/Log.h"

ShaderProgram::ShaderProgram(const std::string& filepath) : m_FilePath(filepath), m_RendererId(0) {

    ShaderProgramSource source = ParseShaderProgram(filepath);

    m_RendererId = CreateShaderProgram(source);
}

void ShaderProgram::Bind() const
{
    glUseProgram(m_RendererId);
}

void ShaderProgram::Unbind() const
{
    glUseProgram(0);
}

void ShaderProgram::SetUniform4f(const std::string& name, const glm::vec4& vec)
{
    glProgramUniform4f(m_RendererId, GetUniformLocation(name), vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform3f(const std::string& name, const glm::vec3& vec)
{
    glProgramUniform3f(m_RendererId, GetUniformLocation(name), vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform2f(const std::string& name, const glm::vec2& vec)
{
    glProgramUniform2f(m_RendererId, GetUniformLocation(name), vec.x, vec.y);
}
void ShaderProgram::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glProgramUniform3f(m_RendererId, GetUniformLocation(name), v0, v1, v2);
}
void ShaderProgram::SetUniform2f(const std::string& name, float v0, float v1)
{
    glProgramUniform2f(m_RendererId, GetUniformLocation(name), v0, v1);
}
void ShaderProgram::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glProgramUniform4f(m_RendererId, GetUniformLocation(name), v0, v1, v2, v3);
}
void ShaderProgram::SetUniform1i(const std::string& name, int value)
{
    glProgramUniform1i(m_RendererId, GetUniformLocation(name), value);
}
void ShaderProgram::SetUniform1f(const std::string& name, float value)
{
    glProgramUniform1f(m_RendererId, GetUniformLocation(name), value);
}
void ShaderProgram::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glProgramUniformMatrix4fv(m_RendererId, GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}
void ShaderProgram::SetUniformMat3f(const std::string& name, const glm::mat3& matrix)
{
    glProgramUniformMatrix3fv(m_RendererId, GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}
void ShaderProgram::SetUniform1fv(const std::string& name, int count, const float* values)
{
    glProgramUniform1fv(m_RendererId, GetUniformLocation(name), count, values);
}

ShaderProgramSource ShaderProgram::ParseShaderProgram(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2
    };

    ShaderType type = ShaderType::NONE;

    std::string line;
    std::stringstream ss[3];

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {

            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
        }
        else {
            ss[(int)type] << line << "\n";
        }
    }
    return { ss[0].str(), ss[1].str(), ss[2].str() };

}

unsigned int ShaderProgram::CreateShaderProgram(const ShaderProgramSource& source)
{
    unsigned int program = glCreateProgram();

    unsigned int vs = AttachShaderToProgram(program, source.VertexSource, GL_VERTEX_SHADER);
    unsigned int fs = AttachShaderToProgram(program, source.FragmentSource, GL_FRAGMENT_SHADER);
    unsigned int gs = 0;
    if (!source.GeometrySource.empty()) {
        gs = AttachShaderToProgram(program, source.GeometrySource, GL_GEOMETRY_SHADER);
    }

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (gs != 0) glDeleteShader(gs);

    return program;
}

unsigned int ShaderProgram::AttachShaderToProgram(unsigned int program, const std::string& source, GLenum type)
{
    unsigned int compiledShader = CompileShader(type, source);
    glAttachShader(program, compiledShader);
    return compiledShader;
}

unsigned int ShaderProgram::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int shader = glCreateShader(type);
    const char* shaderSource = source.c_str();

    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    int result;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));

        glGetShaderInfoLog(shader, length, &length, message);

        SPHERROR("Failed to compile shader!");

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

int ShaderProgram::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_RendererId, name.c_str());
    if (location == -1)

        SPHWARN("Warning: Uniform '" + name + "' doesn't exist!");

    m_UniformLocationCache[name] = location;
    return location;
}

ShaderProgram::~ShaderProgram()
{
    if (m_RendererId != 0) glDeleteProgram(m_RendererId);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
    m_FilePath = std::move(other.m_FilePath);
    m_RendererId = other.m_RendererId;
    m_UniformLocationCache = std::move(other.m_UniformLocationCache);

    other.m_RendererId = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram && other) noexcept
{
    
    if (this == &other) return *this;

    if (m_RendererId != 0) glDeleteProgram(m_RendererId);

    m_FilePath = std::move(other.m_FilePath);
    m_RendererId = other.m_RendererId;
    m_UniformLocationCache = std::move(other.m_UniformLocationCache);

    other.m_RendererId = 0;

    return *this;
}
