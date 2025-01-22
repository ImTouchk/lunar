#version 420 core
	
out vec4 frag_col;

layout (location = 0) in vec3 world_pos;

uniform samplerCube environmentMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfMap;

void main()
{
	//vec3 env_color = texture(skybox, uv).rgb;
	vec3 env_color = textureLod(environmentMap, world_pos, 0.0).rgb;

	env_color = env_color / (env_color + vec3(1.0));
	env_color = pow(env_color, vec3(1.0/2.2));

	frag_col = vec4(env_color, 1.0);
}