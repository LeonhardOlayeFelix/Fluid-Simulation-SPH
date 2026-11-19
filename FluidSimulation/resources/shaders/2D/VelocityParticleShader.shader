#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Instanced attributes
layout(location = 1) in vec3 a_Translation;
layout(location = 2) in vec3 a_Velocity;

layout (std140, binding = 0) uniform Matrices
{
	mat4 u_Proj;
};

uniform float u_BallRadius;
uniform float u_MaxVelocitySize;

out vec2 v_Local;      // position within the particle, -1..1
flat out float v_T;    // normalised speed, 0..1

void main()
{
	v_Local = a_Position.xy;
	v_T = clamp(length(a_Velocity) / u_MaxVelocitySize, 0.0, 1.0);
	gl_Position = u_Proj * vec4(u_BallRadius * a_Position + a_Translation, 1.0);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_Local;
flat in float v_T;

// turns a number in [0,1] into a color by blending 4 colors.
vec3 speedColour(float t)
{
	const vec3 c0 = vec3(0.05, 0.20, 0.55); // deep blue
	const vec3 c1 = vec3(0.10, 0.65, 0.95); // cyan
	const vec3 c2 = vec3(0.85, 0.95, 1.00); // near white
	const vec3 c3 = vec3(1.00, 0.55, 0.20); // warm orange

	t = smoothstep(0.0, 1.0, t);
	if (t < 0.4) 
		return mix(c0, c1, t / 0.4);
	if (t < 0.75) 
		return mix(c1, c2, (t - 0.4) / 0.35);
	else
		return mix(c2, c3, (t - 0.75) / 0.25);
}

void main()
{
	float d = length(v_Local);

	//anti-aliased circular edge
	float edge = fwidth(d);
	float alpha = 1.0 - smoothstep(0.92 - edge, 0.92, d);
	if (alpha <= 0.0) discard;

	vec3 base = speedColour(v_T);

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