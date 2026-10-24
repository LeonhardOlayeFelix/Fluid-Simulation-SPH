#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Translation;

uniform mat4 u_Proj;
uniform float u_BallRadius;

flat out int v_InstanceId;

void main()
{
	gl_Position = u_Proj * vec4(u_BallRadius * a_Position + a_Translation, 1.0f);

	v_InstanceId = gl_InstanceID;
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

uniform int u_SpecimenIndex;

flat in int v_InstanceId;

void main()
{
	if (v_InstanceId == u_SpecimenIndex)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
}