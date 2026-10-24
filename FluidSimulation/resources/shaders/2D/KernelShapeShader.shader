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
	v_Position = a_Position;
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec3 v_Position;

float poly6Alpha(float d)
{
	float d2 = d * d;
	float c = (1 - d2);
	return c * c * c;
}

void main()
{
	float len = length(v_Position);

	if (len > 1)
		discard;

	if (0.98 < len  && len < 1)
		color = vec4(0.0, 0.0, 1.0, 1.0);
	else
		color = vec4(1.0, 1.0, 1.0, poly6Alpha(len));
}