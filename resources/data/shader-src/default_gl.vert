#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in float uv_x;
layout (location = 4) in float uv_y;

out vec2 uv;

void main()
{
	gl_Position = vec4(pos.xyz, 1.0);
	uv = vec2(uv_x, uv_y);
}