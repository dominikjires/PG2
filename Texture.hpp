#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <opencv2\opencv.hpp>
#include <GL/glew.h>

// generate GL texture from image file
GLuint textureInit(const char* filepath);

// generate GL texture from OpenCV image
GLuint tex_gen(cv::Mat& image);

#endif