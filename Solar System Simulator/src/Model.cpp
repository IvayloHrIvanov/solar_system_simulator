#include "Model.h"
#include <cmath>

Model::Model() : VAO(0), VBO(0), EBO(0) {}

Model::~Model() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Model::createSphere(float radius, int sectors, int stacks) {
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / (float)stacks;
        float phi = v * 3.14f;

        for (int j = 0; j <= sectors; ++j) {
            float u = (float)j / ((float)sectors - 0.035f);
            float theta = u * (2.0f * 3.14f);

            Vertex vertex;
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);

            vertex.Position = glm::vec3(x * radius, y * radius, z * radius);
            vertex.Normal = glm::vec3(x, y, z);
            vertex.TexCoords = glm::vec2(u, 1.0f - v);

            vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }

    setupMesh();
}

// Add this function to create ring geometry
void Model::createRing(float innerRadius, float outerRadius, int sectors) {
    vertices.clear();
    indices.clear();

    // Create vertices for the ring
    for (int i = 0; i <= sectors; i++) {
        float angle = 2.0f * 3.14 * i / (sectors - 0.035);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        // Outer circle vertex
        vertices.push_back(Vertex{
            glm::vec3(cosAngle * outerRadius, 0.0f, sinAngle * outerRadius),
            glm::vec3(0.0f, 1.0f, 0.0f),  // Normal pointing up
            glm::vec2(0.0f, (float)i / sectors)  // UV coordinates
            });

        // Inner circle vertex
        vertices.push_back(Vertex{
            glm::vec3(cosAngle * innerRadius, 0.0f, sinAngle * innerRadius),
            glm::vec3(0.0f, 1.0f, 0.0f),  // Normal pointing up
            glm::vec2(1.0f, (float)i / sectors)  // UV coordinates
            });
    }

    // Create indices for triangle strips
    for (int i = 0; i < sectors; i++) {
        int base = i * 2;

        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    // Setup buffers (similar to your sphere creation)
    setupMesh();
}


void Model::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Model::create() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}