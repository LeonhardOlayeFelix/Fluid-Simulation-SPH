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

void main()
{
	vec2 pixelSize = fwidth(v_Position.xy); 

	float leftDistance  = (v_Position.x + 1) / pixelSize.x;
	float rightDistance = (1 - v_Position.x) / pixelSize.x;
	float downDistance  = (v_Position.y + 1) / pixelSize.y;
	float upDistance    = (1 - v_Position.y) / pixelSize.y;

	float distance = min(min(leftDistance, rightDistance), min(downDistance, upDistance));

	if (distance < 1.0)
		color = vec4(0.92, 0.92, 0.92, 1.0);
	else
		discard;
	
}