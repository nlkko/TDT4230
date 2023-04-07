#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;

in layout(location = 3) float height;
in layout(location = 4) float amplitude;

uniform layout(binding = 0) sampler2D noise;

uniform vec3 sun_direction = vec3(6, 2, -10);

uniform layout(location = 5) float time;

#define PI 3.1415926538

// Colors for gradient
uniform vec3 start_color = normalize(vec3(52, 77, 191));
uniform vec3 end_color = normalize(vec3(78, 108, 245));

// BLUE
// vec3(52, 77, 191)
// vec3(78, 108, 245)

uniform float i = 0.05; // intensity
uniform float min_threshold = 0.699;
uniform float max_threshold = 0.7;
uniform float texture_scale = 10;

vec3 gradient(vec3 start_color, vec3 end_color, float value) {
    return vec3(
        start_color.x + (end_color.x - start_color.x) * value,
        start_color.y + (end_color.y - start_color.y) * value,
        start_color.z + (end_color.z - start_color.z) * value
    );
}

out vec4 color;

void main()
{
    float scale = height / amplitude * 0.3;

    vec4 d1 = texture2D(noise, vec2(textureCoordinates.x, textureCoordinates.y - time/150) * texture_scale);
    vec4 d2 = texture2D(noise, vec2(textureCoordinates.x - time/150, textureCoordinates.y) * texture_scale);

    vec4 d_mix = mix(d1, d2, 0.5);

    float angle = dot(normal, sun_direction);
    vec3 gradient_color = gradient(start_color, end_color, scale);

    if (d_mix.z > 0.3 && d_mix.x < 0.3 && d_mix.y < 0.4) {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        //float foam_level = 1 - (textureCoordinates.y / 0.05);
        float foam_level = pow(5, -25 * textureCoordinates.y);
        float foam_osc = sin(time/5);

        if (textureCoordinates.y < 0.15) color = vec4(gradient(start_color, end_color, scale) + vec3(1.0) * foam_level * abs(foam_osc), 1.0);
        else color = vec4(gradient(start_color, end_color, scale), 1.0);

    }

    // Shade
    if (angle > 0.6)
        color.xyz *= 1;
    else if (angle > 0.5)
        color.xyz *= 0.95;
    else if (angle > 0.1)
        color.xyz *= 0.9;
    else
        color.xyz *= 0.85;

    //color = vec4(angle * normal + 0.5, 1.0);
}