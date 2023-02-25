#version 430 core

in vec2 textureCoordinates;

uniform layout(binding = 0) sampler2D tex;

out vec4 color;

void main() {
    color = texture(tex, textureCoordinates) * vec4(1.0, 1.0, 1.0, 1.0);
}