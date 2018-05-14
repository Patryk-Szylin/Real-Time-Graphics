#version 440

uniform sampler2DRect sampler_gPosition;
uniform sampler2DRect sampler_gNormal;
uniform sampler2DRect sampler_gAlbedo;

layout(std140) uniform PerLightUniforms
{
	mat4 combined_xform;

	vec3 light_position;
	float light_range;
	vec3 light_intensity;
};

out vec3 fragment_colour;


float attenuation(vec3 fragmentPosition, vec3 lightPosition, vec3 lightVector, float lightRange)
{
	float dist = distance(fragmentPosition, lightPosition);

	float mix_coefficient = smoothstep(0.0, lightRange, dist);
	float attenuation = 1.0 - mix_coefficient;

	return attenuation;
}

void main(void)
{

	vec3 texel_P = texelFetch(sampler_gPosition, ivec2(gl_FragCoord.xy)).rgb;
	vec3 P = texel_P;

	vec3 texel_N = texelFetch(sampler_gNormal, ivec2(gl_FragCoord.xy)).rgb;
	vec3 N = normalize(texel_N);

	vec3 texel_Color = texelFetch(sampler_gAlbedo, ivec2(gl_FragCoord.xy)).rgb;
	vec3 Color = texel_Color;

	vec3 L = normalize(light_position - P);

	float diff = max(dot(N, L), 0.0);
	vec3 diffuse = light_intensity * diff;

	float attenuation = attenuation(P, light_position, L, light_range);

	fragment_colour = diffuse * Color * attenuation;
}
