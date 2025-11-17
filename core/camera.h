#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

struct Camera {
private:
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    float pitch = 0;
    float yaw = -90;
    glm::mat4 view;
    glm::mat4 projection;

    void UpdateCameraVectors();
public:
    // Construct
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f)) {
        this->position = position;

        this->forward = glm::normalize(target - position);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->right = glm::normalize(glm::cross(worldUp, this->forward));
        this->up = glm::cross(this->forward, this->right);

        this->view = glm::lookAt(this->position, this->position + this->forward, this->up);
    };

    // Copy
    Camera(const Camera& otherCamera) {
        printf("Copying camera. Intended??\n");
        this->position = otherCamera.GetPosition();
        this->forward = otherCamera.GetForward();
        this->view = otherCamera.GetViewMatrix();
        this->up = otherCamera.GetUp();
        this->right = otherCamera.GetRight();
        this->yaw = otherCamera.GetYaw();
        this->pitch = otherCamera.GetPitch();
    }

    // Overload
    Camera& operator=(const Camera& otherCamera) {
        if (this != &otherCamera) {
            this->position = otherCamera.GetPosition();
            this->forward = otherCamera.GetForward();
            this->view = otherCamera.GetViewMatrix();
            this->up = otherCamera.GetUp();
            this->right = otherCamera.GetRight();
            this->yaw = otherCamera.GetYaw();
            this->pitch = otherCamera.GetPitch();
        }
        return *this;
    }

    // Destruct
    ~Camera() {
        printf("Camera deleted\n");
    }

    void MoveCamera(glm::vec3 move);
    void RotateCamera(glm::vec2 rotation);

    glm::mat4 GetProjectionMatrix(float g_width, float g_height) const;
    glm::mat4 GetViewMatrix() const { return this->view; }
    glm::vec3 GetPosition() const { return this->position; }
    glm::vec3 GetForward() const { return this->forward; }
    glm::vec3 GetRight() const { return this->right; }
    glm::vec3 GetUp() const { return this->up; }
    float GetYaw() const { return this->yaw; }
    float GetPitch() const { return this->pitch; }
};