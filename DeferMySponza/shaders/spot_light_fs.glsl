#version 330

out vec3 fragment_colour;

uniform sampler2DRect sampler_gPosition;
uniform sampler2DRect sampler_gNormal;
uniform sampler2DRect sampler_gAlbedo;
uniform sampler2DRect shadowMap;

uniform mat4 lightSpace_xform;
uniform vec3 ambient_intensity;
uniform bool castShadow;

layout(std140) uniform PerLightUniforms
{
	mat4 combined_xform;

	vec3 light_position;
	float light_range;
	vec3 light_intensity;
	float light_angle;
	vec3 light_direction;
};


// Func prototypes
float spotLightAttenuation(vec3 P, vec3 lightPos, float lightRange);
float spotLightConeAttenuation(vec3 lightDir, vec3 L, float coneAngle);

float calculateShadow(vec4 fragPosLightSpace)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	//projCoords = projCoords * 0.5 + 0.5;

	vec2 UVCoords;
	UVCoords.x = (0.5 * projCoords.x + 0.5) * 1024;
	UVCoords.y = (0.5 * projCoords.y + 0.5) * 1024;

	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texelFetch(shadowMap, ivec2(UVCoords)).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = 0.5 + 0.5 * projCoords.z;


	//return (UVCoords.x >= 0 && UVCoords.x < 1024 && UVCoords.y >= 0 && UVCoords.y < 1024) ? 1.0 : 0.0;
	//return smoothstep(0.99, 1.0, currentDepth);

	// Check whether current frag pos is in shadow

	float bias = 0.005;
	float shadow = currentDepth  > closestDepth ? 1.0 : 0.0;

	return shadow;



}

void main(void)
{

	vec3 texel_P = texelFetch(sampler_gPosition, ivec2(gl_FragCoord.xy)).rgb;
	vec3 P = texel_P;

	vec3 texel_N = texelFetch(sampler_gNormal, ivec2(gl_FragCoord.xy)).rgb;
	vec3 N = normalize(texel_N);

	vec3 texel_Color = texelFetch(sampler_gAlbedo, ivec2(gl_FragCoord.xy)).rgb;
	vec3 Color = texel_Color;

	vec4 shadow_pos = lightSpace_xform * vec4(P, 1.0f);

	vec3 L = normalize(light_position - P);

	// Ambient
	vec3 ambient = ambient_intensity * light_intensity;

	// Diffuse


	// Diffuse component
	float diff = max(dot(N, L), 0.0);
	vec3 diffuse = light_intensity * diff;

	float coneAttenuation = spotLightConeAttenuation(light_direction, L, light_angle);
	float attenuation = spotLightAttenuation(P, light_position, light_range);

	// Shadow factor
	float shadowFactor = castShadow ? calculateShadow(shadow_pos) : 1.0;

	fragment_colour = (ambient + (1.0f - shadowFactor) * (diffuse  * attenuation ))* coneAttenuation;
}


float spotLightAttenuation(vec3 P, vec3 lightPos, float lightRange)
{
	float dist = distance(P, lightPos);
	float mix_coefficient = smoothstep(0.0, lightRange, dist * 2);
	float att = 1 - mix_coefficient;

	if (dist > lightRange)
	{
		att = 0;
	}

	return att;
}

float spotLightConeAttenuation(vec3 lightDir, vec3 L, float coneAngle)
{
	float arc_light_range = degrees(acos(dot(lightDir, -L)));		

	if (arc_light_range > coneAngle / 2)
		return 0;

	return smoothstep(coneAngle / 2, coneAngle / 4, arc_light_range);
}