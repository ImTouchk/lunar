#version 420 core
out vec4 frag_col;

layout (location = 0) in vec2 uv;

uniform sampler2D source;

void main()
{		
	frag_col = texture(source, uv);
}