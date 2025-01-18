#version 420 core
layout (location = 0) in vec3 in_pos;
layout (location = 3) in float in_uv_x;
layout (location = 4) in float in_uv_y;

out vec2 uv;

void main()
{		
	uv = vec2(in_uv_x, in_uv_y);
	gl_Position = vec4(in_pos, 1);
}