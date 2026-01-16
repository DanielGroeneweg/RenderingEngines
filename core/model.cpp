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
    }

    void Model::rotate(glm::vec3 axis, float radians) {
        modelMatrix = glm::rotate(modelMatrix, radians, axis);
    }

    void Model::scale(glm::vec3 scale) {
        modelMatrix = glm::scale(modelMatrix, scale);
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
}