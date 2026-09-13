#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
	gl_Position = vec4(a_Position.xyz, 1.0f);
	v_TexCoords = a_TexCoords;
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoords;

uniform sampler2D u_QuadTexture;

void main()
{
	color = texture(u_QuadTexture, v_TexCoords);
}