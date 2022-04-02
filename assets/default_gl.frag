#version 330 core

out vec4 color;

in vec2 texture_uv;
uniform sampler2D mytex;

void main()
{
	color = texture(mytex, texture_uv);
}