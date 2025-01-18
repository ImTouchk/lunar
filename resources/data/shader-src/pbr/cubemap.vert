#version 420 core
layout (location = 0) in vec3 in_pos;

uniform mat4 projection;
uniform mat4 view;

out vec3 local_pos;

void main()
{		
	local_pos = in_pos;
	gl_Position = projection * view * vec4(local_pos, 1.0);
}