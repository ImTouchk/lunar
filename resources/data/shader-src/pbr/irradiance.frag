#version 420 core
	
out vec4 frag_col;

in vec3 local_pos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
	vec3 N          = normalize(local_pos);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up         = normalize(cross(N, right));

    float sample_delta = 0.025;
    float sample_count = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sample_delta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sample_delta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangent_sample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N; 

            irradiance += texture(environmentMap, sample_vec).rgb * cos(theta) * sin(theta);
            sample_count++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(sample_count));

	frag_col = vec4(irradiance, 1.0);
}
