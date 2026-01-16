#pragma once

#include <string>
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include "mesh.h"

namespace core {
    class Model {
    private:
        std::vector<core::Mesh> meshes;
        glm::mat4 modelMatrix;
        glm::vec3 baseColor;
        glm::vec3 translation;
        std::string modelName;
    public:
        Model(std::vector<core::Mesh> meshes, glm::vec3 color = glm::vec3(1,1,1), glm::vec3 translation = glm::vec3(0,0,0), std::string name = "model") : meshes(meshes), modelMatrix(1), baseColor(color), translation(translation), modelName(name) {}

        void render();

        void translate(glm::vec3 translation);
        void rotate(glm::vec3 axis, float radians);
        void scale(glm::vec3 scale);
        const glm::vec3 GetBaseColor();
        void SetBaseColor(glm::vec3 color);
        const glm::vec3 GetTranslation();
        const std::string GetName();
        void SetName(std::string name);
        glm::mat4 getModelMatrix() const;
    };
}