#version 420 core
	
out vec4 frag_col;

in vec3 local_pos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(local_pos)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    frag_col = vec4(color, 1.0);
}
