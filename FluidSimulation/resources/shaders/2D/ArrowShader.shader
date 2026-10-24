#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Translation;
layout(location = 3) in vec3 a_Gradient;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_Proj;
};

uniform float u_ArrowLengthMax;
uniform float u_MaxDensityGradientSize;
uniform float u_ArrowWidth;
uniform bool u_NegateGradient;

void main()
{

	if (dot(a_Gradient, a_Gradient) < 1e-12f) 
	{
		gl_Position = vec4(0.0);
		return;
	}

	float length = u_ArrowLengthMax * length(a_Gradient) / u_MaxDensityGradientSize;

	vec3 scale = vec3(length / 2, u_ArrowWidth / 2, 0);

	vec3 translate1 = vec3(length / 2, 0, 0);

	float theta = atan(a_Gradient.y, a_Gradient.x) + (u_NegateGradient ? 3.14159 : 0);
	float c = cos(theta);
	float s = sin(theta);
	mat3 rotation = mat3(c, s, 0.0, -s, c, 0.0, 0.0, 0.0, 1.0);

	vec3 position = rotation * (scale * a_Position + translate1) + a_Translation;

	gl_Position = u_Proj * vec4(position, 1.0f);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
