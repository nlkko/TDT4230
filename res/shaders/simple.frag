#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;

// TODO: Getting error "non constant expression for array size" if i try to send value from CPU, any circumvention?
const int N_LIGHTS = 3;
uniform layout(location = 7) vec3 light_position[N_LIGHTS];
uniform layout(location = 100) vec4 test2;

const vec3 test_light_position = vec3(0.0);

const float ambient_strength = 0.1;
const vec3 ambient_color = vec3(1.0);

const float diffuse_strength = 0.3;
const vec3 diffuse_color = vec3(1.0);

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    vec3 normalized_normal = normalize(normal);

    float diffuse_intensity = 0;

    for (int i = 0; i < N_LIGHTS; i++) {
        vec3 light_direction = normalize( vec3(light_position[i] - position) ) ;
        diffuse_intensity += max( dot ( normalized_normal, light_direction ), 0.0);
    }

    vec3 ambient = ambient_strength * ambient_color;
    vec3 diffuse = diffuse_strength * diffuse_color * diffuse_intensity;

    color = vec4(diffuse + ambient, 1.0);
}