#version 330 core

out vec4 color;

in vec3 vertex_pos;

void main()
{
	color = vec4(vertex_pos.xyz, 1);
}