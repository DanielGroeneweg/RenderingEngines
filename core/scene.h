#pragma once
#include <vector>
#include "model.h"
#include <string>

#include "camera.h"

struct Scene {
private:
    std::vector<core::Model*> objects;
    std::string sceneName;
    Camera camera;
public:
    // Constructor
    Scene(std::string sceneName) {
        this-> sceneName = sceneName;
        this-> camera = Camera();
    }

    // Copy
    Scene(const Scene& otherScene) = delete;

    // Overload
    Scene& operator=(const Scene& otherScene) = delete;

    // Destructor
    ~Scene() {
        printf("Scene %s Deleted\n", this->GetSceneName().c_str());
    }

    void AddObject(core::Model* object);

    std::vector<core::Model*> GetObjects() const {return this->objects;}
    std::string GetSceneName() const {return this->sceneName;}
    Camera* GetCamera() {return &(this->camera);}
};