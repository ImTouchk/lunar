#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 tex_uv;

layout (push_constant) uniform constants
{
	mat4 transform;
};

void main()
{
	gl_Position = transform * vec4(pos.xyz, 1);
	gl_Position.y = -gl_Position.y;

	tex_uv = uv;
}

