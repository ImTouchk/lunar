#version 420 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in float uv_x;
layout (location = 4) in float uv_y;

layout (std140, binding = 0) uniform SceneData
{
	mat4 projection;
	mat4 view;
	mat4 model;
};

out vec2 uv;

void main()
{
	gl_Position = projection * view * model * vec4(pos.xyz, 1.0);
	uv = vec2(uv_x, uv_y);
}