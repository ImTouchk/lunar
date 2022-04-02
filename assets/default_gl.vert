#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_uv;

uniform mat4 transform;

out vec2 texture_uv;

void main()
{
	gl_Position = transform * vec4(pos.xyz, 1);
	texture_uv = tex_uv;
}