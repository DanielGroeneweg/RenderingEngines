#include "camera.h"

#include <ios>
#include <glm/gtc/quaternion.hpp>

void Camera::MoveCamera(glm::vec3 move) {
    glm::vec3 relativeMove =
          move.z * this->forward    // W/S movement
        + move.x * right            // A/D movement
        + move.y * this->up;        // Q/E movement
    this->position += relativeMove;
    this->view = glm::lookAt(this->position, this->position + this->forward, this->up);
}
void Camera::RotateCamera(glm::vec2 rotation) {
    this->yaw += rotation.x;
    this->pitch += rotation.y;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateCameraVectors();

    this->view = glm::lookAt(this->position, this->position + this->forward, this->up);
}
glm::mat4 Camera::GetProjectionMatrix(float g_width, float g_height) const {
    return glm::perspective(glm::radians(45.0f), g_width / g_height, 0.1f, 100.0f);
}
void Camera::UpdateCameraVectors() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    dir.y = sin(glm::radians(this->pitch));
    dir.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    this->forward = glm::normalize(dir);
    this->right = glm::normalize(glm::cross(this->forward, worldUp));
    this->up = glm::normalize(glm::cross(this->right, this->forward));
}