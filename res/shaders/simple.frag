#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;

// TODO: Getting error "non constant expression for array size" if i try to send value from CPU, any circumvention?
const int N_LIGHTS = 3;


//uniform layout(location = 7) vec3 light_position[N_LIGHTS];

// TODO: Send array, instead of individual locations to the shader.
uniform layout(location = 7) vec3 light0;
uniform layout(location = 8) vec3 light1;
uniform layout(location = 9) vec3 light2;

uniform layout(location = 10) vec3 camera_position;

const vec3 light_color = vec3(1.0);

const float ambient_strength = 0.1;
const float diffuse_strength = 0.3;
const float specular_strength = 0.6;
const int specular_factor = 32; // 2, 4, 8, 16, 32, 64, 128, 256

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    vec3 normalized_normal = normalize(normal);

    vec3 light_position[3];
    light_position[0] = light0;
    light_position[1] = light1;
    light_position[2] = light2;

    float diffuse_intensity = 0;
    float specular_shine = 0;

    for (int i = 0; i < N_LIGHTS; i++) {
        vec3 light_direction = normalize( light_position[i] - position ) ;
        vec3 view_direction = normalize( camera_position - position );
        vec3 reflection_direction = reflect(-light_direction, normalized_normal);

        specular_shine += pow( max( dot( view_direction, reflection_direction ), 0.0 ), specular_factor);

        diffuse_intensity += max( dot ( normalized_normal, light_direction ), 0.0);
    }

    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = diffuse_strength * diffuse_intensity * light_color;
    vec3 specular = specular_strength * specular_shine * light_color;

    color = vec4(ambient + diffuse + specular, 1.0);
}