#include <iostream>
#include "glfont.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    // Normalised stride/width for u
    // Specify value between 0 and 1,
    float u_stride = 1 / float(text.length());

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        // Such that (texCoord.x ∗ texImageWidth) = desiredPixelCoord.x
        float base_u_texture_coordinate = float(i) * u_stride;

        // Some values are assigned the same value again - redundant or necessary?

        // Triangle - 90 degrees vertex bottom right
        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0}; // u_0
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0}; // u
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0}; // v

        // Triangle - 90 degrees vertex top left
        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0}; // u_0
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0}; // v
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0}; // u


        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

        mesh.textureCoordinates.at(4 * i + 0) = {base_u_texture_coordinate, 0};
        mesh.textureCoordinates.at(4 * i + 1) = {base_u_texture_coordinate + u_stride, 0};
        mesh.textureCoordinates.at(4 * i + 2) = {base_u_texture_coordinate + u_stride, 1};

        mesh.textureCoordinates.at(4 * i + 0) = {base_u_texture_coordinate, 0};
        mesh.textureCoordinates.at(4 * i + 2) = {base_u_texture_coordinate + u_stride, 0};
        mesh.textureCoordinates.at(4 * i + 3) = {base_u_texture_coordinate, 1};
    }

    return mesh;
}