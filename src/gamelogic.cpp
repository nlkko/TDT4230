#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"
#include "utilities/textures.h"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double viewpointX = 0;
double viewpointZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* padNode;
SceneNode* textNode;

SceneNode* skyboxNode;
SceneNode* waveNode;
SceneNode* boatNode;
SceneNode* coastNode;

glm::mat4 VP;

unsigned int noise_texture_id;

const int N_LIGHTS = 3;

glm::vec3 cameraPosition;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shader_2d;
Gloom::Shader* shader_skybox;
Gloom::Shader* shader_wave;
Gloom::Shader* shader_object;

sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

CommandLineOptions options;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    //glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    viewpointX -= mouseSensitivity * deltaX / windowWidth;
    viewpointZ -= mouseSensitivity * deltaY / windowHeight;

    if (viewpointX > 1) viewpointX = 1;
    if (viewpointX < 0) viewpointX = 0;
    if (viewpointZ > 1) viewpointZ = 1;
    if (viewpointZ < 0) viewpointZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

struct LightSource {
    SceneNode* node;
    glm::vec3 position;
    glm::vec3 color;
};

LightSource lightSources[N_LIGHTS];

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {

    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");

    shader_2d = new Gloom::Shader();
    shader_2d->makeBasicShader("../res/shaders/simple2d.vert", "../res/shaders/simple2d.frag");

    shader_skybox = new Gloom::Shader();
    shader_skybox->makeBasicShader("../res/shaders/skybox.vert", "../res/shaders/skybox.frag");

    shader_wave = new Gloom::Shader();
    shader_wave->makeBasicShader("../res/shaders/wave.vert", "../res/shaders/wave.frag");

    shader_object = new Gloom::Shader();
    shader_object->makeBasicShader("../res/shaders/object.vert", "../res/shaders/object.frag");


    // Create meshes
    Mesh testCube = cube(glm::vec3(50, 20, 20), glm::vec2(40, 40), true);
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);

    Mesh skybox = cube(glm::vec3(400, 400, 400), glm::vec2(90), true, true);
    Mesh wave = generatePlane(500, glm::vec2(700, 700));

    std::cout << "Objects:" << std::endl;
    Mesh boat = load_obj("../res/objects/boat.obj");
    Mesh coast = load_obj("../res/objects/coast.obj");
    std::cout << std::endl;

    // Text Texture
    float char_width = 29.0;
    float char_height = 39.0;
    std::string displayed_text = "";
    float mesh_width = displayed_text.length() * char_width;

    PNGImage charmap = loadPNGFile("../res/textures/charmap.png"); // Load textures
    unsigned int texture_id = generateTexture(&charmap); // Generate textures
    Mesh text_mesh = generateTextGeometryBuffer(displayed_text, char_height / char_width, mesh_width); // Generate text

    // Object Textures
    PNGImage boat_texture = loadPNGFile("../res/textures/objects/boat.png");
    PNGImage coast_texture = loadPNGFile("../res/textures/objects/coastline.png");

    // Noise Texture
    PNGImage noise = loadPNGFile("../res/textures/noise/cloud_noise.png");
    noise_texture_id = generateTexture(&noise);

    // Box Texture
    PNGImage box_diffuse_image = loadPNGFile("../res/textures/Brick03_col.png");
    PNGImage box_normal_image = loadPNGFile("../res/textures/Brick03_nrm.png");

    unsigned int box_texture_id = generateTexture(&box_diffuse_image);
    unsigned int box_normal_map_id = generateTexture(&box_normal_image);

    // Skybox Texture

    std::string skybox_type = "day2";

    std::vector<PNGImage> faces;
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/right.png", skybox_type)));
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/left.png", skybox_type)));
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/top.png", skybox_type)));
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/bottom.png", skybox_type)));
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/back.png", skybox_type)));
    faces.push_back(loadPNGFile(fmt::format("../res/textures/{}/front.png", skybox_type)));

    // Create lights
    for (int i = 0; i < N_LIGHTS; i++) {
        lightSources[i].node = createSceneNode();
        lightSources[i].node->vertexArrayObjectID = i;
        lightSources[i].node->nodeType = POINT_LIGHT;
    }

    // Light positions
    lightSources[0].node->position = glm::vec3(60.0, 15.0, -15.0);
    lightSources[1].node->position = glm::vec3(-60.0, 15.0, -15.0);
    lightSources[2].node->position = glm::vec3(0.0, 40, 0.0);

    // Light colors
    lightSources[0].color = glm::vec3(1.0, 0.0, 0.0);
    lightSources[1].color = glm::vec3(0.0, 1.0, 0.0);
    lightSources[2].color = glm::vec3(1.0, 1.0, 1.0);

    // Fill buffers
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(text_mesh);
    unsigned int testCubeVAO = generateBuffer(testCube);

    unsigned int skyboxVAO  = generateBuffer(skybox);
    unsigned int boatVAO    = generateBuffer(boat);
    unsigned int waveVAO    = generateBuffer(wave);
    unsigned int coastVAO   = generateBuffer(coast);

    // Construct scene
    rootNode     = createSceneNode();
    skyboxNode   = createSceneNode();
    waveNode     = createSceneNode();
    boatNode     = createSceneNode();
    coastNode    = createSceneNode();

    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    textNode = createSceneNode();

    // NodeType
    textNode    ->nodeType = GEOMETRY_2D;
    boxNode     ->nodeType = NORMAL_MAPPED_GEOMETRY;
    skyboxNode  ->nodeType = SKYBOX;
    waveNode    ->nodeType = WAVE;
    boatNode    ->nodeType = OBJECT;
    coastNode   ->nodeType = OBJECT;

    // Properties
    skyboxNode  ->texture_id = generateCubemap(faces);
    boatNode    ->texture_id = generateTexture(&boat_texture);
    coastNode   ->texture_id = generateTexture(&coast_texture);

    textNode    ->texture_id = texture_id;

    boxNode     ->texture_id = box_texture_id;
    boxNode     ->normal_map_texture_id = box_normal_map_id;

    // Push
    rootNode    ->children.push_back(skyboxNode);



    rootNode    ->children.push_back(coastNode);
    rootNode    ->children.push_back(waveNode);
    rootNode    ->children.push_back(boatNode);


    //rootNode->children.push_back(boxNode);
    //rootNode    ->children.push_back(padNode);
    //rootNode    ->children.push_back(textNode);

    // Stationary Lights
    boxNode     ->children.push_back(lightSources[0].node);
    boxNode     ->children.push_back(lightSources[1].node);




    // Dynamic Lights
    padNode     ->children.push_back(lightSources[2].node);

    // VAO
    skyboxNode->vertexArrayObjectID = skyboxVAO;
    skyboxNode->VAOIndexCount       = skybox.indices.size();

    boatNode->vertexArrayObjectID   = boatVAO;
    boatNode->VAOIndexCount         = boat.indices.size();

    waveNode->vertexArrayObjectID  = waveVAO;
    waveNode->VAOIndexCount        = wave.indices.size();

    coastNode->vertexArrayObjectID = coastVAO;
    coastNode->VAOIndexCount       = coast.indices.size();

    boxNode->vertexArrayObjectID    = boxVAO;
    boxNode->VAOIndexCount          = box.indices.size();

    padNode->vertexArrayObjectID    = padVAO;
    padNode->VAOIndexCount          = pad.indices.size();

    textNode->vertexArrayObjectID   = textVAO;
    textNode->VAOIndexCount         = text_mesh.indices.size();

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    for (int i = 0; i < N_LIGHTS - 1; ++i) {
        std::cout << fmt::format("Light #{} | Position: {}, {}, {}", i, lightSources[i].node->position[0], lightSources[i].node->position[0], lightSources[i].node->position[0]) << std::endl;
    }

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    gameElapsedTime += getTimeDeltaSeconds();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 1000.f);

    cameraPosition = glm::vec3(0, -38, -20);
    //cameraPosition += glm::vec3(1, 0, 0);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (viewpointX - 0.5))) + 0.3;
    glm::mat4 cameraTransform =
            glm::rotate(0.3f + 0.2f * float(-viewpointZ * viewpointZ), glm::vec3(1, 0, 0)) *
            glm::rotate(lookRotation, glm::vec3((cos(gameElapsedTime)/6), 0.9, (sin(gameElapsedTime)/3))) *
            glm::translate(-cameraPosition);


    /*
     // unlocked view
     glm::mat4 cameraTransform = glm::rotate(float(-viewpointZ), glm::vec3(1, 0, 0)) *
     glm::rotate(float(-viewpointX), glm::vec3(0, 1, 0)) *
     glm::translate(-cameraPosition);
    */

    VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    skyboxNode->position = cameraPosition;


    boatNode->position   = glm::vec3(0.0, -58.0, -26.0);
    boatNode->rotation   = glm::vec3((cos(gameElapsedTime)/12), -0.7, (sin(gameElapsedTime)/10));
    boatNode->scale      = glm::vec3(20);

    waveNode->position   = glm::vec3(80.0, -60.0, -1500.0);
    waveNode->scale      = glm::vec3(2);
    waveNode->rotation  = glm::vec3(0, -0.5, 0);

    coastNode->position  = glm::vec3(-150.0, -55.0, -190.0);
    coastNode->scale     = glm::vec3(1);
    coastNode->rotation  = glm::vec3(0, -0.5, 0);

    boxNode->position    = { 0, -10, -80 };
    textNode->position   = glm::vec3(0.0, 0.0, 0.0);







    //waveNode->rotation += glm::vec3(getTimeDeltaSeconds() / 2, 0, 0);

    /*
    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };
     */

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - viewpointX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - viewpointZ) * (boxDimensions.z - padDimensions.z)
    };

    updateNodeTransformations(rootNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);


    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;
    node->MVP = VP * node->currentTransformationMatrix;

    node->normal = glm::mat3( transpose( inverse( node->currentTransformationMatrix ) ) );

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: {
            glm::vec4 origin = glm::vec4(0.0, 0.0, 0.0, 1.0);
            lightSources[node->vertexArrayObjectID].position = glm::vec3(node->currentTransformationMatrix * origin);
            break;
        }

        case SPOT_LIGHT: break;
        case GEOMETRY_2D: break;
        case NORMAL_MAPPED_GEOMETRY: break;
        case SKYBOX: break;
        case WAVE: break;
        case OBJECT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {

    if (node->nodeType != GEOMETRY_2D && node->nodeType != SKYBOX) {
        //shader->activate();

        // M
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

        // MVP
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->MVP));

        // Normal
        glUniformMatrix3fv(5, 1, GL_FALSE, glm::value_ptr(node->normal));

        // Camera Position
        glUniform3fv(10, 1, glm::value_ptr(cameraPosition));
    }

    if (node->nodeType != NORMAL_MAPPED_GEOMETRY && node->nodeType != SKYBOX) {
        // Flag for normal mapping feature
        glUniform1i(13, GL_FALSE);
    }

    switch(node->nodeType) {

        case GEOMETRY: {
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        }

        case POINT_LIGHT: {
            if (node->vertexArrayObjectID != -1) {
                GLint location_position = shader->getUniformFromName(fmt::format("light_sources[{}].position", node->vertexArrayObjectID));
                glUniform3fv(location_position, 1, glm::value_ptr(lightSources[node->vertexArrayObjectID].position));

                GLint location_color = shader->getUniformFromName(fmt::format("light_sources[{}].color", node->vertexArrayObjectID));
                glUniform3fv(location_color, 1, glm::value_ptr(lightSources[node->vertexArrayObjectID].color));
            }
            break;
        }

        case SPOT_LIGHT: break;


        case GEOMETRY_2D:{
            if (node->vertexArrayObjectID != -1) {
                shader_2d->activate();

                // Orthographic projection
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight))));

                // Binding texture
                glBindTextureUnit(0, node->texture_id);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }

            break;
        }


        case NORMAL_MAPPED_GEOMETRY: {
            if (node->vertexArrayObjectID != -1) {
                // Flag for normal mapping feature
                glUniform1i(13, GL_TRUE);

                // Binding textures
                glBindTextureUnit(0, node->texture_id);
                glBindTextureUnit(1, node->normal_map_texture_id);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }

            break;
        }


        case SKYBOX: {
            if (node->vertexArrayObjectID != -1) {
                shader_skybox->activate();
                glDepthMask(GL_FALSE);

                // MVP
                glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->MVP));

                // Skybox Texture
                glBindTextureUnit(GL_TEXTURE_CUBE_MAP, node->texture_id);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
                glDepthMask(GL_TRUE);
                shader_skybox->deactivate();
            }

            break;
        }

        case WAVE: {
            if (node->vertexArrayObjectID != -1) {
                shader_wave->activate();

                // MVP
                glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->MVP));

                // Total Elapsed Time
                glUniform1f(5, gameElapsedTime);

                // Noise
                glBindTextureUnit(0, noise_texture_id);

                glBindVertexArray(node->vertexArrayObjectID);

                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
                shader_wave->deactivate();
            }

            break;
        }

        case OBJECT: {
            if (node->vertexArrayObjectID != -1) {
                shader_object->activate();

                // MVP
                glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->MVP));

                // Total Elapsed Time
                glUniform1f(5, gameElapsedTime);

                // Texture
                glBindTextureUnit(0, node->texture_id);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

                shader_object->deactivate();
            }

            break;
        }
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    renderNode(rootNode);
}
