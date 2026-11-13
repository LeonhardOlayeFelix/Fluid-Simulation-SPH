#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_Proj;
	mat4 u_View;
};

void main()
{
	gl_Position = u_Proj * u_View * vec4(a_Position, 1.0f);
}

#shader fragment
#version 450 core
layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}