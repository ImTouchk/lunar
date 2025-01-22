#version 420 core
layout (location = 0) in vec3 in_pos;

uniform mat4 projection;
uniform mat4 view;

layout (location = 0) out vec3 world_pos;

void main()
{
	world_pos = in_pos;

	mat4 rot_view = mat4(mat3(view)); // remove translation
	vec4 clip_pos = projection * rot_view * vec4(world_pos, 1.0);
	gl_Position = clip_pos.xyww;
}
