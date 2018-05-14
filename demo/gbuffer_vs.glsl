#version 440
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_texCoords;
layout(location = 3) in mat4 model_matrix;

uniform mat4 projection_xform;
uniform mat4 view_xform;

out VERTEX
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
} vs_out;


void main(void)
{
	mat4 final_xform = projection_xform * view_xform * model_matrix;

	gl_Position = final_xform * vec4(vertex_position, 1.0);

	vs_out.position = mat4x3(model_matrix) * vec4(vertex_position, 1.0f);
	vs_out.texCoords = vertex_texCoords;
	vs_out.normal = mat3(model_matrix) * vertex_normal;
}
