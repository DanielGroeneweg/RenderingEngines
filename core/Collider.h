#pragma once
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include "mesh.h"

class Collider {
public:
    struct Bounds {
        glm::vec3 min;
        glm::vec3 max;
    };

private:
    Bounds localBounds;
    Bounds worldBounds;
    glm::vec3 size;

public:
    Collider(const std::vector<core::Mesh>& meshes) {
        ComputeLocalBounds(meshes);
    }

    const Bounds& GetWorldBounds() const { return worldBounds; }

    void Update(const glm::mat4& modelMatrix);

    const glm::vec3& GetSize() { return size; }

private:
    void ComputeLocalBounds(const std::vector<core::Mesh>& meshes);
};