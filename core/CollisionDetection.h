#pragma once
#include <vector>
#include "model.h"
#include "collider.h"

class CollisionDetection {
public:
    static void FindCollisions(std::vector<core::Model*> models);
private:
    static bool SATCollision(std::vector<core::Model*> models);
};