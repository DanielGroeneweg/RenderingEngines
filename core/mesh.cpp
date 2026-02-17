#include "mesh.h"

namespace core {
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices) : vertices(vertices), indices(indices) {
        setupBuffers();
    }

    void Mesh::setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(Vertex) * vertices.size()), &vertices[0],
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(unsigned int) * indices.size()),
                     &indices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));
        glBindVertexArray(0);
    }

    Mesh Mesh::generateQuad() {
        const glm::vec3 pos[] = {
                glm::vec3(-1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
        };

        const glm::vec3 normals[] = {
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
        };
        const glm::vec2 uvs[] = {
                glm::vec2(0, 0),
                glm::vec2(1, 0),
                glm::vec2(0, 1),
                glm::vec2(1, 0),
                glm::vec2(1, 1),
                glm::vec2(0, 1)
        };
        const std::vector<GLuint> indices = {0, 1, 2, 3, 4, 5};

        std::vector<Vertex> vertexVector;
        vertexVector.reserve(6);
        for (int i = 0; i < 6; ++i) {
            vertexVector.emplace_back(pos[i], normals[i], uvs[i]);
        }

        return Mesh(vertexVector, indices);
    }

    Mesh Mesh::generateScreenQuad() {
        float quadVerts[] = {
            // positions   // texcoords
            -1.0f,  1.0f,   0.0f, 1.0f,
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,

            -1.0f,  1.0f,   0.0f, 1.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f
        };

        std::vector<Vertex> v;
        v.reserve(6);

        for (int i = 0; i < 6; ++i) {
            glm::vec3 p(quadVerts[i*4+0], quadVerts[i*4+1], 0);
            glm::vec3 n(0,0,1);
            glm::vec2 uv(quadVerts[i*4+2], quadVerts[i*4+3]);
            v.emplace_back(p, n, uv);
        }

        std::vector<GLuint> idx = {0,1,2,3,4,5};
        return Mesh(v, idx);
    }

    void Mesh::render() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    }
    const std::vector<Vertex>& Mesh::getVertices() const {
        return vertices;
    }
    std::vector<Mesh::Triangle> Mesh::getTriangles() const {
        std::vector<Triangle> tris;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            Triangle t;
            t.p0 = vertices[indices[i]].position;
            t.p1 = vertices[indices[i+1]].position;
            t.p2 = vertices[indices[i+2]].position;

            tris.push_back(t);
        }

        return tris;
    }

}