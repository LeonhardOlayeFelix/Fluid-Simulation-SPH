#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

//Instanced attributes
layout(location = 1) in vec3 a_Translation;
layout(location = 2) in vec3 a_Velocity;

layout (std140, binding = 0) uniform Matrices
{
	mat4 u_Proj;
};

uniform float u_BallRadius;
uniform float u_MaxVelocitySize;

flat out float v_T;

void main()
{
	gl_Position = u_Proj * vec4(u_BallRadius * a_Position + a_Translation, 1.0f);
	v_T = min(length(a_Velocity) / u_MaxVelocitySize, 1.0);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

flat in float v_T;


void main()
{
	

	color = vec4(v_T, (1 - v_T) * 1.0f, (1 - v_T) * 1.0f, 1.0f);
}