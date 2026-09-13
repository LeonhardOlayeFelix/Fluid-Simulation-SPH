#shader vertex
#version 450 core

layout(location = 0) in vec4 a_Position;

void main()
{
	gl_Position = vec4(a_Position.xyz, 1.0f);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}