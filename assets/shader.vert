#version 450

layout (location = 0) in vec3 pos;

layout (push_constant) uniform constants
{
	mat4 transform;
};

void main()
{
	gl_Position = transform * vec4(pos.xyz, 1);
	gl_Position.y = -gl_Position.y;
}

