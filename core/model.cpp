#include "model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace core {
    void Model::render() {
        for (int i = 0; i < meshes.size(); ++i) {
            meshes[i].render();
        }
    }

    void Model::translate(glm::vec3 translation) {
        modelMatrix = glm::translate(modelMatrix, translation);
        this->translation += translation;
        collider.Update(modelMatrix);
    }

    void Model::rotate(glm::vec3 axis, float radians) {
        modelMatrix = glm::rotate(modelMatrix, radians, axis);
        collider.Update(modelMatrix);
    }

    void Model::scale(glm::vec3 scale) {
        modelMatrix = glm::scale(modelMatrix, scale);
        collider.Update(modelMatrix);
    }

    glm::mat4 Model::getModelMatrix() const {
        return this->modelMatrix;
    }
    const glm::vec3 Model::GetBaseColor() {
        return this->baseColor;
    }
    void Model::SetBaseColor(glm::vec3 color) {
        this->baseColor = color;
    }
    const glm::vec3 Model::GetTranslation() {
        return this->translation;
    }
    const std::string Model::GetName() {
        return this->modelName;
    }
    void Model::SetName(std::string name) {
        this->modelName = name;
    }
    Collider Model::GetCollider() {
        return this->collider;
    }
    std::vector<core::Mesh::Triangle> Model::GetTriangles() const {
        std::vector<core::Mesh::Triangle> triangles;

        for (core::Mesh mesh : meshes) {
            for (core::Mesh::Triangle triangle : mesh.getTriangles()) {
                triangles.push_back(triangle);
            }
        }

        return triangles;
    }
}