#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec3 position;

uniform samplerCube skybox;

out vec4 color;

void main()
{
    color = texture(skybox, position);
}