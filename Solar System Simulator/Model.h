#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Model {
public:
    Model();
    ~Model();
    void create();
    void createSphere(float radius, int sectors, int stacks);
    void createRing(float innerRadius, float outerRadius, int sectors);

private:
    GLuint VAO, VBO, EBO;
    vector<Vertex> vertices;
    vector<GLuint> indices;

    void setupMesh();
};

#endif