#version 450

layout (location = 0) in vec2 tex_uv;
layout (location = 0) out vec4 pixel;

layout (binding = 1) uniform sampler2D tex_sampler;

void main()
{
	pixel = texture(tex_sampler, tex_uv);
}

