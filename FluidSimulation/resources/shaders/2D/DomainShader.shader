#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_Proj;
};

uniform mat4 u_Model;

out vec3 v_Position;

void main()
{
	gl_Position = u_Proj * u_Model * vec4(a_Position, 1.0f);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0, 0.0, 0.0, 0.2);
}