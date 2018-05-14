#version 330
layout(location = 0) in vec3 vertex_position;
layout(location = 3) in mat4 model_matrix;

uniform mat4 lightSpace_xform;

void main(void)
{
	gl_Position = lightSpace_xform * model_matrix * vec4(vertex_position, 1.0);
}
