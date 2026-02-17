#include "CollisionDetection.h"

#include <array>
#include <algorithm>
#include "model.h"
#include "collider.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"
void CollisionDetection::FindCollisions(std::vector<core::Model*> models) {
    for (int i = 0; i < models.size(); i++) {
        Collider::Bounds model1box = models[i]->GetCollider().GetWorldBounds();

        for (int j = i+1; j < models.size(); j++) {
            Collider::Bounds model2box = models[j]->GetCollider().GetWorldBounds();

            // Check X axis
            if (model1box.max.x < model2box.min.x || model1box.min.x > model2box.max.x)
                continue;

            // Check Y axis
            if (model1box.max.y < model2box.min.y || model1box.min.y > model2box.max.y)
                continue;

            // Check Z axis
            if (model1box.max.z < model2box.min.z || model1box.min.z > model2box.max.z)
                continue;

            // BOX Collision is here
            {
                printf("%s's bounds collided with %s\n's bounds", models[i]->GetName().c_str(), models[j]->GetName().c_str());

                std::vector<core::Model*> collidingModels;
                collidingModels.push_back(models[i]);
                collidingModels.push_back(models[j]);

                if (SATCollision(collidingModels)) printf("%s's mesh collided with %s\n's mesh", models[i]->GetName().c_str(), models[j]->GetName().c_str());
            }
        }
    }
}
bool CollisionDetection::SATCollision(std::vector<core::Model*> models) {
    auto trianglesA = models[0]->GetTriangles();
    auto trianglesB = models[1]->GetTriangles(); // use [1], not [0]

    for (const auto& t1 : trianglesA) {
        for (const auto& t2 : trianglesB) {

            auto edges1 = std::array<glm::vec3, 3>{ t1.p1 - t1.p0, t1.p2 - t1.p1, t1.p0 - t1.p2 };
            auto edges2 = std::array<glm::vec3, 3>{ t2.p1 - t2.p0, t2.p2 - t2.p1, t2.p0 - t2.p2 };

            glm::vec3 normal1 = glm::cross(edges1[0], edges1[1]);
            glm::vec3 normal2 = glm::cross(edges2[0], edges2[1]);

            std::vector<glm::vec3> axes;
            if (glm::length2(normal1) > 1e-6f) axes.push_back(glm::normalize(normal1));
            if (glm::length2(normal2) > 1e-6f) axes.push_back(glm::normalize(normal2));

            // cross products of edges
            for (const auto& e1 : edges1) {
                for (const auto& e2 : edges2) {
                    glm::vec3 axis = glm::cross(e1, e2);
                    if (glm::dot(axis, axis) > 1e-6f) // use dot for squared length
                        axes.push_back(glm::normalize(axis));
                }
            }

            auto project = [](const glm::vec3& axis, const core::Mesh::Triangle& tri) {
                float p0 = glm::dot(axis, tri.p0);
                float p1 = glm::dot(axis, tri.p1);
                float p2 = glm::dot(axis, tri.p2);
                return std::make_pair(std::min({p0,p1,p2}), std::max({p0,p1,p2}));
            };

            bool separated = false;

            for (const auto& axis : axes) {
                auto [min1, max1] = project(axis, t1);
                auto [min2, max2] = project(axis, t2);
                if (max1 < min2 || max2 < min1) {
                    separated = true; // found a separating axis
                    break; // no collision for this triangle pair
                }
            }

            if (!separated) {
                // no separating axis -> collision detected, early exit
                return true;
            }
        }
    }

    // all triangle pairs have a separating axis -> no collision
    return false;
}
