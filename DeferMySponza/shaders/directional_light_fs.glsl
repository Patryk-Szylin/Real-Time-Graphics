#version 440
#define NR_DIR_LIGHTS 2

const float PI = 3.14159265f;

uniform sampler2DRect sampler_gPosition;
uniform sampler2DRect sampler_gNormal;
uniform sampler2DRect sampler_gAlbedo;

uniform vec3 ambient_intensity;
uniform vec3 light_direction[2];
uniform vec3 light_intensity[2];
uniform vec3 camera_position;


out vec4 fragment_colour;


float DotClamp(vec3 A, vec3 B)
{
	float Result = max(0.0f, dot(A, B));
	return(Result);
}

vec3 DiffuseColor(vec3 objectColor, vec3 lightColor, vec3 LightDir, vec3 Normal)
{
	vec3 result = objectColor * lightColor * max(0.0, dot(LightDir, Normal));
	return result;
}


float fresnelSchlick(float F0, float LdotH)
{
	return F0 + (1.0 - F0) * pow(1.0 - LdotH, 5.0);
}


// --------------------------------------------------
// GGX BRDF
// --------------------------------------------------

float GGXDistributionTerm(float AlphaSqr, float NdotH)
{
	float Denominator = ((NdotH * NdotH) * (AlphaSqr - 1.0f)) + 1.0f;
	Denominator = PI * Denominator * Denominator;
	float Result = AlphaSqr / Denominator;

	return(Result);
}

float GGXBRDF(vec3 N, vec3 lightDir, vec3 halfDir, vec3 viewDir, float Alpha, float F0)
{
	float NdotH = DotClamp(N, halfDir);
	float NdotL = DotClamp(N, lightDir);
	float NdotV = DotClamp(N, viewDir);
	float VdotH = DotClamp(viewDir, halfDir);
	float LdotH = DotClamp(lightDir, halfDir);

	float AlphaSqr = Alpha * Alpha;

	float F = fresnelSchlick(F0, LdotH);
	float D = GGXDistributionTerm(AlphaSqr, NdotH);

	float OneOverGL = NdotL + sqrt(AlphaSqr + ((1.0f - AlphaSqr) * (NdotL * NdotL)));
	float OneOverGV = NdotV + sqrt(AlphaSqr + ((1.0f - AlphaSqr) * (NdotV * NdotV)));

	float diffuseFactor = 0.3f;
	float result = diffuseFactor + ((F + D) / OneOverGL * OneOverGV);

	return (result);
}


void main()
{
	// Retrive data from G-Buffer
	/////////////////////////////

	vec3 texel_P = texelFetch(sampler_gPosition, ivec2(gl_FragCoord.xy)).rgb;
	vec3 P = texel_P;

	vec3 texel_N = texelFetch(sampler_gNormal, ivec2(gl_FragCoord.xy)).rgb;
	vec3 N = normalize(texel_N);

	vec3 texel_Color = texelFetch(sampler_gAlbedo, ivec2(gl_FragCoord.xy)).rgb;
	vec3 Color = texel_Color;

	vec3 result = vec3(0.0, 0.0, 0.0);

	vec3 reflectionColor = Color;

	vec3 testAmbient = vec3(0.1f, 0.1f, 0.1f);

	//vec3 viewDir = normalize(P - camera_position);
	vec3 viewDir = normalize(-P);


	for (int i = 0; i < NR_DIR_LIGHTS; i++)
	{
		vec3 lightDir = normalize(light_direction[i]);
		vec3 halfDir = normalize(viewDir + lightDir);

		float BRDF = GGXBRDF(N, lightDir, halfDir, viewDir, 0.1f, 0.9f); // Rougness and Metallness values 0.1 : 0.9
		vec3 Li = ambient_intensity * reflectionColor * light_intensity[i];

		vec3 temp = vec3(BRDF, 0.0, 0.0);

		result += (light_intensity[i] * reflectionColor * testAmbient) * BRDF;

		//result += BRDF * Li * DotClamp(N, lightDir);
	}



	fragment_colour = vec4(result, 1.0f);





}