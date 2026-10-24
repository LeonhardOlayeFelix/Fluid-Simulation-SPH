#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Translation;

uniform mat4 u_Proj;
uniform vec3 u_SpecimenPosition;

flat out int v_InstanceId;

uniform float u_SmoothingRadiusSquared;
uniform float u_BallRadius;

flat out int v_WithinRadius;

void main()
{

	gl_Position = u_Proj * vec4(u_BallRadius * a_Position + a_Translation, 1.0f);

	v_InstanceId = gl_InstanceID;

	vec3 d = a_Translation - u_SpecimenPosition; 
	v_WithinRadius = dot(d, d) <= u_SmoothingRadiusSquared ? 1 : 0;
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

uniform int u_SpecimenIndex;

flat in int v_InstanceId;
flat in int v_WithinRadius;

void main()
{

	if (v_InstanceId == u_SpecimenIndex)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (v_WithinRadius == 1)
	{
		color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else
	{
		color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
}