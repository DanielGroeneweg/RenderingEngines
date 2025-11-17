#include "scene.h"

void Scene::AddObject(core::Model* object) {
    this->objects.push_back(object);
}