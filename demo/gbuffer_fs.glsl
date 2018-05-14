#version 440
layout(location = 0) out vec3 gBuffer_Position;
layout(location = 1) out vec3 gBuffer_Normal;
layout(location = 2) out vec4 gBuffer_Albedo;


in VERTEX
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
} fragment;


uniform vec3 albedo_color;
uniform float albedo_shininess;

void main(void)
{
	gBuffer_Position = fragment.position;
	gBuffer_Normal = normalize(fragment.normal);
	gBuffer_Albedo.rgb = albedo_color;
	gBuffer_Albedo.a = albedo_shininess;
}
