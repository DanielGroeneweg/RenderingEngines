#pragma once

#include <vector>
#include <glad/glad.h>
#include "vertex.h"

namespace core {
    class Mesh {
    public:
        struct Triangle {
            glm::vec3 p0;
            glm::vec3 p1;
            glm::vec3 p2;
        };
    private:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
    public:
        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices);
        static Mesh generateScreenQuad();
        void render();
        static Mesh generateQuad();
        const std::vector<Vertex>& getVertices() const;
        std::vector<Triangle> getTriangles() const;
    private:
        void setupBuffers();
    };
}