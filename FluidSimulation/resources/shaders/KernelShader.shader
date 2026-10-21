#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_Proj;

out vec3 v_Position;

void main()
{
	gl_Position = u_Proj * u_Model * vec4(a_Position, 1.0f);
	v_Position = a_Position;
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec3 v_Position;

void main()
{
	float alpha = 0.5f;
	if (length(v_Position) > 1)
		discard;

	color = vec4(1.0, 1.0, 1.0, alpha);
}