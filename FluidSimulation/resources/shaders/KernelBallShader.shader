#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;
uniform mat4 u_Proj;

void main()
{
	gl_Position = u_Proj * u_Model * vec4(a_Position, 1.0f);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

uniform bool u_IsSpecimen;
uniform bool u_WithinRadius;

void main()
{
	if (u_IsSpecimen)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (u_WithinRadius)
	{
		color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else
	{
		color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
}