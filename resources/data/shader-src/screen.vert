#version 420 core
layout (location = 0) in vec3 in_pos;
layout (location = 3) in float in_uv_x;
layout (location = 4) in float in_uv_y;

layout (location = 0) out vec2 out_uv;

void main()
{
	out_uv      = vec2(in_uv_x, in_uv_y);
	//out_normal  = transpose(inverse(mat3(model))) * in_normal;
	gl_Position = vec4(in_pos.xyz, 1.0);
}
