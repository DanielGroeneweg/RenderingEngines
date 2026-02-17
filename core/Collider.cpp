#include "Collider.h"
void Collider::ComputeLocalBounds(const std::vector<core::Mesh>& meshes)
{
    localBounds.min = glm::vec3(std::numeric_limits<float>::max());
    localBounds.max = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& mesh : meshes)
    {
        for (const auto& v : mesh.getVertices())
        {
            localBounds.min = glm::min(localBounds.min, v.position);
            localBounds.max = glm::max(localBounds.max, v.position);
        }
    }

    worldBounds = localBounds;
}
void Collider::Update(const glm::mat4& modelMatrix)
{
    glm::vec3 min = localBounds.min;
    glm::vec3 max = localBounds.max;

    glm::vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z},
    };

    worldBounds.min = glm::vec3(std::numeric_limits<float>::max());
    worldBounds.max = glm::vec3(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed =
            glm::vec3(modelMatrix * glm::vec4(corners[i], 1.0f));

        worldBounds.min = glm::min(worldBounds.min, transformed);
        worldBounds.max = glm::max(worldBounds.max, transformed);
    }

    this->size.x = worldBounds.max.x - worldBounds.min.x;
    this->size.y = worldBounds.max.y - worldBounds.min.y;
    this->size.z = worldBounds.max.z - worldBounds.min.z;
}