#pragma once
#include <string>
#include <unordered_map>

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
	std::string GeometrySource;
};

class ShaderProgram
{
	std::string m_FilePath{};
	unsigned int m_RendererId{};
	std::unordered_map<std::string, int> m_UniformLocationCache{};

};

