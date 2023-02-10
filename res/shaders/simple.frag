#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

uniform layout(location = 6) int N_LIGHTS;

const float ambient_strength = 0.75;
const vec3 ambient_color = vec3(1.0);

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    color = vec4(0.5 * normalize(normal) + 0.5, 4.0);
}