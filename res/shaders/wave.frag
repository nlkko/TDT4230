#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;

in layout(location = 3) float height;
in layout(location = 4) float amplitude;

uniform layout(binding = 0) sampler2D tex_noise1;
uniform layout(binding = 1) sampler2D tex_noise2;

uniform layout(location = 5) float time;

// Colors for gradient
uniform vec3 start_color = normalize(vec3(87, 105, 187));
uniform vec3 end_color = vec3(0.772, 1.0, 0.952);
uniform float i = 0.05; // intensity
uniform float min_threshold = 0.699;
uniform float max_threshold = 0.7;

vec3 gradient(vec3 start_color, vec3 end_color, float value) {
    return vec3(
        start_color.x + (end_color.x - start_color.x) * value * i,
        start_color.y + (end_color.y - start_color.y) * value * i,
        start_color.z + (end_color.z - start_color.z) * value * i
    );
}

out vec4 color;

void main()
{
    float scale = height / amplitude;

    vec3 n1 = texture2D(tex_noise1, vec2(textureCoordinates.x, textureCoordinates.y + time / 10)).xyz;
    vec3 n2 = texture2D(tex_noise2, vec2(textureCoordinates.x + time/20, textureCoordinates.y)).xyz;

    vec3 n = n1 + n2;

    // sqrt( pow(n2.x - n1.x, 2) + pow(n2.y - n1.y, 2) + pow(n2.z - n1.z, 2))

    float dist = 1 - sqrt( pow(n2.x - n1.x, 2) + pow(n2.y - n1.y, 2) + pow(n2.z - n1.z, 2));

    if( dist > min_threshold && dist < max_threshold) {
        color = vec4(1.0, 1.0, 1.0, 0.8);
    } else color = vec4(gradient(start_color, end_color, scale), 0.8);


}