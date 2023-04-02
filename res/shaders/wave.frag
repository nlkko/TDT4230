#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;

in layout(location = 3) float height;
in layout(location = 4) float amplitude;

// Colors for gradient
uniform vec3 start_color = normalize(vec3(87, 105, 187));
uniform vec3 end_color = vec3(0.772, 1.0, 0.952);
uniform float i = 0.05; // intensity

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
    color = vec4(gradient(start_color, end_color, scale), 0.8);
}