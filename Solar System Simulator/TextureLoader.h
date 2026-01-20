#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

GLuint loadTextureFromFile(const char* path);
GLuint createDefaultTexture(glm::vec3 color);
#endif