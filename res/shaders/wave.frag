#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

out vec4 color;

void main()
{
    color = vec4(0.2 * textureCoordinates.x, 0.871 * textureCoordinates.y, 0.929 * textureCoordinates.x, 0.8);
}