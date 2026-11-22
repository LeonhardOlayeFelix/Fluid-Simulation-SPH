#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(location = 1) in vec3 a_Translation;

layout (std140, binding = 0) uniform Matrices
{
	mat4 u_Proj;
};

uniform float u_BallRadius;

out vec2 v_Local;

void main()
{
	v_Local = a_Position.xy;
	gl_Position = u_Proj * vec4(u_BallRadius * a_Position + a_Translation, 1.0);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_Local;

void main()
{
	float d = length(v_Local);

	//anti-aliased circular edge
	float edge = fwidth(d);
	float alpha = 1.0 - smoothstep(0.92 - edge, 0.92, d);
	if (alpha <= 0.0) discard;

	vec3 base = vec3(0.05, 0.20, 0.55);

	//fake sphere shading: brighter towards the upper-left
	float z = sqrt(max(1.0 - d * d, 0.0));
	vec3 normal = normalize(vec3(v_Local, z));
	vec3 lightDir = normalize(vec3(-0.4, 0.5, 0.8));
	float diffuse = 0.55 + 0.45 * max(dot(normal, lightDir), 0.0);

	//small specular highlight
	vec3 halfDir = normalize(lightDir + vec3(0.0, 0.0, 1.0));
	float spec = pow(max(dot(normal, halfDir), 0.0), 40.0) * 0.6;

	//slightly darker rim for definition
	float rim = smoothstep(0.55, 0.92, d);
	vec3 shaded = base * diffuse * (1.0 - 0.25 * rim) + vec3(spec);

	color = vec4(shaded, alpha);
	//color = vec4(1.0, 1.0, 1.0, 1.0);
}