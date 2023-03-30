#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

struct LightSource {
    vec3 position;
    vec3 color;
};

const int N_LIGHTS = 3;
uniform LightSource light_sources[N_LIGHTS];

uniform layout(location = 10) vec3 camera_position;

const float ambient_strength = 0.9;
const float diffuse_strength = 0.8;
const float specular_strength = 0.05;
const int specular_factor = 2; // 2, 4, 8, 16, 32, 64, 128, 256

// Attenuation
const float constant = 0.001;
const float linear = 0.0001;
const float quadratic = 0.001;

// Texture
uniform layout(binding = 0) sampler2D geometry_texture;
uniform layout(binding = 1) sampler2D geometry_normal_map;

// Flag for normal mapped geometry
uniform layout(location = 13) bool normal_mapped_geometry;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }
vec3 reject(vec3 from, vec3 onto) { return from - onto*dot(from, onto)/dot(onto, onto); }

void main()
{
    vec3 normalized_normal;

    if (normal_mapped_geometry) {
        normalized_normal = TBN * (texture(geometry_normal_map, textureCoordinates).xyz * 2 - 1);
    } else {
        normalized_normal = normalize(normal);
    }
    vec3 diffuse_intensity = vec3(0.0);
    vec3 specular_shine = vec3(0.0);
    vec3 light_color = vec3(0.0);
    float attenuation = 0;

    for (int i = 0; i < N_LIGHTS; i++) {
        // Attenuation
        float light_distance = length( light_sources[i].position - position );
        attenuation = 1.0 / (constant + light_distance * linear + pow( light_distance, 2) * quadratic );

        // Lighting
        vec3 light_direction = normalize( light_sources[i].position - position );
        vec3 view_direction = normalize( camera_position - position );
        vec3 reflection_direction = reflect( -light_direction, normalized_normal );

        vec3 texture_color;
        if (normal_mapped_geometry) {
            texture_color = texture(geometry_texture, textureCoordinates).xyz;
        } else {
            texture_color = vec3(1.0);
        }

        // Add up calculations
        specular_shine += pow( max( dot( view_direction, reflection_direction ), 0.0 ), specular_factor) * attenuation * light_sources[i].color;
        diffuse_intensity += max( dot ( normalized_normal, light_direction ), 0.0) * attenuation * light_sources[i].color * texture_color;
    }

    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = diffuse_strength * diffuse_intensity;
    vec3 specular = specular_strength * specular_shine;

    color = vec4(ambient + diffuse + specular + dither(textureCoordinates), 1.0);
}