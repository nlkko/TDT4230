#ifndef TEXTURES_H
#define TEXTURES_H
#pragma once

#include <glad/glad.h>
#include <fmt/format.h>
#include "imageLoader.hpp"

unsigned int generateTexture(PNGImage *image);
unsigned int generateCubemap(std::vector<PNGImage> faces);

#endif