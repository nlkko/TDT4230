#version 430 core

in layout(location = 0) vec3 position_in;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 14) vec3 tangent;
in layout(location = 15) vec3 bitangent;

uniform layout(location = 3) mat4 M;
uniform layout(location = 4) mat4 MVP;
uniform layout(location = 5) mat3 normal;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 position_out;
out layout(location = 3) mat3 TBN_out;


void main()
{

    normal_out = normalize(normal * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    position_out = vec3( M * vec4(position_in, 1.0) );

    TBN_out = mat3(normalize(normal * tangent), normalize(normal * bitangent), normal_out);
    gl_Position = MVP * vec4(position_in, 1.0);
}
