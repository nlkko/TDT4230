#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 4) mat4 MVP;
uniform layout(location = 5) float elapsed_time;


out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;

void main()
{

    mat4 wave_function = mat4(
    1,	        0,			0,			0,
    0,			1,      	0,			0,
    0,			0,			1,      	0,
    0,			sin(position.x + elapsed_time * 2),			0,			1
    );

    normal_out = normal_in;
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = MVP * wave_function * vec4(position, 1.0);

}