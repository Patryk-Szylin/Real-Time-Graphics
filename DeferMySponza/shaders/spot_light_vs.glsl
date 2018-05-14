#version 440
layout(location = 0) in vec3 vertex_position;


layout(std140) uniform PerLightUniforms
{
	mat4 combined_xform;

	vec3 light_position;
	float light_range;
	vec3 light_intensity;
	float light_angle;
	vec3 light_direction;
};

void main(void)
{
	gl_Position = combined_xform * vec4(vertex_position, 1.0);
}
