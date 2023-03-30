#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

out vec4 color;

void main()
{
    color = vec4(0.5 * normal + 0.5, 1.0);
}