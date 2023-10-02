#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

uniform layout(binding = 0) sampler2D t; // Texture
uniform vec3 sun_direction = vec3(-0.5, 3, 0);

out vec4 color;

void main()
{
    //color = vec4(0.5 * normal + 0.5, 1.0);
    color = texture(t, textureCoordinates);


    float angle = dot(normal, sun_direction);

    // shade
    if (angle > 0.9)
    color.xyz *= 1;
    else if (angle > 0.6)
    color.xyz *= 0.9;
    else if (angle > 0.4)
    color.xyz *= 0.85;
    else
    color.xyz *= 0.7;
}