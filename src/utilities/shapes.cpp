#include <iostream>
#include "shapes.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

Mesh cube(glm::vec3 scale, glm::vec2 textureScale, bool tilingTextures, bool inverted, glm::vec3 textureScale3d) {
    glm::vec3 points[8];
    int indices[36];

    for (int y = 0; y <= 1; y++)
    for (int z = 0; z <= 1; z++)
    for (int x = 0; x <= 1; x++) {
        points[x+y*4+z*2] = glm::vec3(x*2-1, y*2-1, z*2-1) * 0.5f * scale;
    }

    int faces[6][4] = {
        {2,3,0,1}, // Bottom 
        {4,5,6,7}, // Top 
        {7,5,3,1}, // Right 
        {4,6,0,2}, // Left 
        {5,4,1,0}, // Back 
        {6,7,2,3}, // Front 
    };

    scale = scale * textureScale3d;
    glm::vec2 faceScale[6] = {
        {-scale.x,-scale.z}, // Bottom
        {-scale.x,-scale.z}, // Top
        { scale.z, scale.y}, // Right
        { scale.z, scale.y}, // Left
        { scale.x, scale.y}, // Back
        { scale.x, scale.y}, // Front
    }; 

    glm::vec3 normals[6] = {
        { 0,-1, 0}, // Bottom 
        { 0, 1, 0}, // Top 
        { 1, 0, 0}, // Right 
        {-1, 0, 0}, // Left 
        { 0, 0,-1}, // Back 
        { 0, 0, 1}, // Front 
    };

    glm::vec2 UVs[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };

    Mesh m;
    for (int face = 0; face < 6; face++) {
        int offset = face * 6;
        indices[offset + 0] = faces[face][0];
        indices[offset + 3] = faces[face][0];

        if (!inverted) {
            indices[offset + 1] = faces[face][3];
            indices[offset + 2] = faces[face][1];
            indices[offset + 4] = faces[face][2];
            indices[offset + 5] = faces[face][3];
        } else {
            indices[offset + 1] = faces[face][1];
            indices[offset + 2] = faces[face][3];
            indices[offset + 4] = faces[face][3];
            indices[offset + 5] = faces[face][2];
        }

        for (int i = 0; i < 6; i++) {
            m.vertices.push_back(points[indices[offset + i]]);
            m.indices.push_back(offset + i);
            m.normals.push_back(normals[face] * (inverted ? -1.f : 1.f));
        }

        glm::vec2 textureScaleFactor = tilingTextures ? (faceScale[face] / textureScale) : glm::vec2(1);

        if (!inverted) {
            for (int i : {1,2,3,1,0,2}) {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        } else {
            for (int i : {3,1,0,3,0,2}) {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        }
    }

    return m;
}

Mesh generateSphere(float sphereRadius, int slices, int layers) {
    const unsigned int triangleCount = slices * layers * 2;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<glm::vec2> uvs;

    vertices.reserve(3 * triangleCount);
    normals.reserve(3 * triangleCount);
    indices.reserve(3 * triangleCount);

    // Slices require us to define a full revolution worth of triangles.
    // Layers only requires angle varying between the bottom and the top (a layer only covers half a circle worth of angles)
    const float degreesPerLayer = 180.0 / (float) layers;
    const float degreesPerSlice = 360.0 / (float) slices;

    unsigned int i = 0;

    // Constructing the sphere one layer at a time
    for (int layer = 0; layer < layers; layer++) {
        int nextLayer = layer + 1;

        // Angles between the vector pointing to any point on a particular layer and the negative z-axis
        float currentAngleZDegrees = degreesPerLayer * layer;
        float nextAngleZDegrees = degreesPerLayer * nextLayer;

        // All coordinates within a single layer share z-coordinates.
        // So we can calculate those of the current and subsequent layer here.
        float currentZ = -cos(glm::radians(currentAngleZDegrees));
        float nextZ = -cos(glm::radians(nextAngleZDegrees));

        // The row of vertices forms a circle around the vertical diagonal (z-axis) of the sphere.
        // These radii are also constant for an entire layer, so we can precalculate them.
        float radius = sin(glm::radians(currentAngleZDegrees));
        float nextRadius = sin(glm::radians(nextAngleZDegrees));

        // Now we can move on to constructing individual slices within a layer
        for (int slice = 0; slice < slices; slice++) {

            // The direction of the start and the end of the slice in the xy-plane
            float currentSliceAngleDegrees = slice * degreesPerSlice;
            float nextSliceAngleDegrees = (slice + 1) * degreesPerSlice;

            // Determining the direction vector for both the start and end of the slice
            float currentDirectionX = cos(glm::radians(currentSliceAngleDegrees));
            float currentDirectionY = sin(glm::radians(currentSliceAngleDegrees));

            float nextDirectionX = cos(glm::radians(nextSliceAngleDegrees));
            float nextDirectionY = sin(glm::radians(nextSliceAngleDegrees));

            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * radius * nextDirectionX,
                                  sphereRadius * radius * nextDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * nextRadius * currentDirectionX,
                                  sphereRadius * nextRadius * currentDirectionY,
                                  sphereRadius * nextZ);

            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(radius * nextDirectionX,
                                 radius * nextDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(nextRadius * currentDirectionX,
                                 nextRadius * currentDirectionY,
                                 nextZ);

            indices.emplace_back(i + 0);
            indices.emplace_back(i + 1);
            indices.emplace_back(i + 2);
            indices.emplace_back(i + 3);
            indices.emplace_back(i + 4);
            indices.emplace_back(i + 5);

            for (int j = 0; j < 6; j++) {
                glm::vec3 vertex = vertices.at(i+j);
                uvs.emplace_back(
                    0.5 + (glm::atan(vertex.z, vertex.y)/(2.0*M_PI)),
                    0.5 - (glm::asin(vertex.y)/M_PI)
                );
            }

            i += 6;
        }
    }

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.textureCoordinates = uvs;
    return mesh;
}

// Tesselation is how many tesselations in one side
Mesh generatePlane(int tesselation, glm::vec2 size) {
    // Create vertices
    Mesh plane;

    float row_stride = size.x / float(tesselation-1);
    float col_stride = size.y / float(tesselation-1);

    float row_uv_stride = 1 / float(tesselation-1);
    float col_uv_stride = 1 / float(tesselation-1);

    // Vertices:
    // The plane is in 3D, however given vector is 2D, which is why y is placed in the z-axis.

    // UV:
    // Texture goes from 0 to 1, essentially 1 /
    for (int row = 0; row < tesselation; ++row) {
        for (int col = 0; col < tesselation; ++col) {
            plane.vertices.push_back(glm::vec3(col * col_stride, 0.0, row * row_stride));
            plane.textureCoordinates.push_back(glm::vec2(row * row_uv_stride, col * col_uv_stride));
            plane.normals.push_back(glm::vec3(0, 1, 0));
        }
    }

    // Create indices
    for (int row = 0; row < tesselation - 1; ++row) {
        for (int col = 0; col < tesselation - 1; ++col) {
            int i = (row*tesselation) + col;

            // Bottom Left Triangle
            plane.indices.push_back(i);
            plane.indices.push_back(i + tesselation );
            plane.indices.push_back(i + 1);

            // Top Left Triangle
            plane.indices.push_back(i + 1);
            plane.indices.push_back(i + tesselation);
            plane.indices.push_back(i + tesselation + 1);
        }
    }

    // Debug
    if (false) {
        std::cout << "Vertices\n";
        int count = 0;
        for (auto vertex : plane.vertices) {
            count++;
            std::cout << vertex.x  << ' ' << vertex.z;

            if (count%4==0) {
                std::cout << '\n';
            } else {
                std::cout << " | ";
            }
        }

        std::cout << "UV Coords\n";
        count = 0;
        for (auto vertex : plane.textureCoordinates) {
            count++;
            std::cout << vertex.x  << ' ' << vertex.y;

            if (count%4==0) {
                std::cout << '\n';
            } else {
                std::cout << " | ";
            }
        }

        count = 0;
        std::cout << "Indices\n";
        for (auto index : plane.indices) {
            count++;
            std::cout << index << ' ';

            if (count%3==0) {
                std::cout << '\n';
            }
        }
    }

    return plane;
}

Mesh load_obj(const std::string &file) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string error;
    std::string warning;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file.c_str(), nullptr, true);

    if (!ret) {exit(1);}

    if (shapes.size() > 1){
        std::cerr << "More than 1 mesh in the file" << std::endl;
    }

    if (shapes.empty()){
        std::cerr << "File does not contain meshes" << std::endl;
    }

    const auto &shape = shapes.front();
    std::cout << "   " << shape.name << " | " << "File: " << file << std::endl;
    const auto &tmesh = shape.mesh;

    Mesh object;

    for (const auto idx : tmesh.indices) {
        object.indices.push_back(object.indices.size());
        glm::vec3 vertex;
        vertex.x = attrib.vertices.at(3*idx.vertex_index+0);
        vertex.y = attrib.vertices.at(3*idx.vertex_index+1);
        vertex.z = attrib.vertices.at(3*idx.vertex_index+2);
        object.vertices.push_back(vertex);
        glm::vec3 normal;
        normal.x = attrib.normals.at(3*idx.normal_index+0);
        normal.y = attrib.normals.at(3*idx.normal_index+1);
        normal.z = attrib.normals.at(3*idx.normal_index+2);
        object.normals.push_back(normal);
        glm::vec3 uv;
        uv.x = attrib.texcoords.at(2*idx.texcoord_index+0);
        uv.y = attrib.texcoords.at(2*idx.texcoord_index+1);
        object.textureCoordinates.push_back(uv);
    }

    return object;
}

