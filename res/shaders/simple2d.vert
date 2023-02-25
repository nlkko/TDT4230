#version 430 core

in layout(location = 0) vec3 position_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 ortho;

out vec2 textureCoordinates;

void main() {
    textureCoordinates = textureCoordinates_in;
    gl_Position = ortho * vec4(position_in, 1.0);
}