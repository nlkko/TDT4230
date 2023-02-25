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

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;
SceneNode* textNode;

double ballRadius = 3.0f;

glm::mat4 VP;

const int N_LIGHTS = 3;

glm::vec3 cameraPosition;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shader_2d;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

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
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}


unsigned int generateTexture(PNGImage *image) {
    // Reserving ID for texture
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load into GPU's VRAM
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels.data());

    // Generate mipmaps for texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // Address oversampling and undersampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
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

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);


    // Text Texture
    float char_width = 29.0;
    float char_height = 39.0;
    std::string displayed_text = "UNSUSPICIOUS BASEMENT";
    float mesh_width = displayed_text.length() * char_width;

    PNGImage charmap = loadPNGFile("../res/textures/charmap.png"); // Load textures
    unsigned int texture_id = generateTexture(&charmap); // Generate textures
    Mesh text_mesh = generateTextGeometryBuffer(displayed_text, char_height / char_width, mesh_width); // Generate text

    // Box Texture
    PNGImage box_diffuse_image = loadPNGFile("../res/textures/Brick03_col.png");
    PNGImage box_normal_image = loadPNGFile("../res/textures/Brick03_nrm.png");

    unsigned int box_texture_id = generateTexture(&box_diffuse_image);
    unsigned int box_normal_map_id = generateTexture(&box_normal_image);

    // Create lights
    for (int i = 0; i < N_LIGHTS; i++) {
        lightSources[i].node = createSceneNode();
        lightSources[i].node->vertexArrayObjectID = i;
        lightSources[i].node->nodeType = POINT_LIGHT;
    }

    // Light positions
    lightSources[0].node->position = glm::vec3(60.0, 15.0, -15.0);
    lightSources[1].node->position = glm::vec3(-60.0, 15.0, -15.0);
    lightSources[2].node->position = glm::vec3(0.0, 10, 0.0);

    // Light colors
    lightSources[0].color = glm::vec3(1.0, 0.0, 0.0);
    lightSources[1].color = glm::vec3(0.0, 1.0, 0.0);
    lightSources[2].color = glm::vec3(1.0, 1.0, 1.0);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(text_mesh);

    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    ballNode = createSceneNode();
    textNode = createSceneNode();

    // NodeType
    textNode->nodeType = GEOMETRY_2D;
    boxNode->nodeType = NORMAL_MAPPED_GEOMETRY;

    // Attributes
    textNode->position = glm::vec3(0.0, 0.0, 0.0);
    textNode->texture_id = texture_id;

    boxNode->texture_id = box_texture_id;
    boxNode->normal_map_texture_id = box_normal_map_id;

    // Push
    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);
    rootNode->children.push_back(textNode);

    // Stationary Lights
    //boxNode->children.push_back(lightSources[0].node);
    //boxNode->children.push_back(lightSources[1].node);

    // Dynamic Lights
    ballNode->children.push_back(lightSources[2].node);

    boxNode->vertexArrayObjectID  = boxVAO;
    boxNode->VAOIndexCount        = box.indices.size();

    padNode->vertexArrayObjectID  = padVAO;
    padNode->VAOIndexCount        = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount       = sphere.indices.size();

    textNode->vertexArrayObjectID = textVAO;
    textNode->VAOIndexCount = text_mesh.indices.size();

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    for (int i = 0; i < N_LIGHTS - 1; ++i) {
        std::cout << fmt::format("Light #{} | Position: {}, {}, {}", i, lightSources[i].node->position[0], lightSources[i].node->position[0], lightSources[i].node->position[0]) << std::endl;
    }

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

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

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ
                ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform =
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);

    VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
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
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    if (node->nodeType != GEOMETRY_2D) {
        shader->activate();

        // M
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

        // MVP
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->MVP));

        // Normal
        glUniformMatrix3fv(5, 1, GL_FALSE, glm::value_ptr(node->normal));

        // Camera Position
        glUniform3fv(10, 1, glm::value_ptr(cameraPosition));

        // Ball Values
        glUniform3fv(11, 1, glm::value_ptr(ballPosition));
        glUniform1f(12, ballRadius);
    }

    if (node->nodeType != NORMAL_MAPPED_GEOMETRY) {
        // Flag for normal mapping feature
        glUniform1i(13, GL_FALSE);
    }

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
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

                glm::mat4 orthographic = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(orthographic));

                glBindTextureUnit(0, node->texture_id);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }

            break;
        };
        case NORMAL_MAPPED_GEOMETRY: {
            // Flag for normal mapping feature
            glUniform1i(13, GL_TRUE);

            // Binding textures
            glBindTextureUnit(0, node->texture_id);
            glBindTextureUnit(1, node->normal_map_texture_id);


            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
        };
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
