#version 430 core

in layout(location = 0) vec3 position_in;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 M;
uniform layout(location = 4) mat4 MVP;
uniform layout(location = 5) mat3 normal;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 position_out;


void main()
{

    normal_out = normalize(normal * normal_in);
    textureCoordinates_out = textureCoordinates_in;

    position_out = M * vec4(position_in, 1.0);
    gl_Position = MVP * vec4(position_in, 1.0);
}
