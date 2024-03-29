#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal; 
out vec3 v_positon;

uniform mat4 u_modelViewProj;
uniform mat4 u_modelView;
uniform mat4 u_invModelView;

void main()
{
	gl_Position = u_modelViewProj * vec4(a_position, 1.0f);
	v_normal = mat3(u_invModelView) * a_normal;
	v_positon = vec3(u_modelView * vec4(a_position, 1.0f));
}