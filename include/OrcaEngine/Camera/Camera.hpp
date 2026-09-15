#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    ~Camera();


    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(float aspect_ratio);

    void SetPosition(glm::vec3& position);
    void SetRotation(float yaw, float pitch);

    void MoveForward(float amount);
    void MoveRight(float amount);
    void MoveUp(float amount);
    void AddYaw(float amount);
    void AddPitch(float amount);

private:

    void UpdateDirectionVectors();

    glm::vec3 _position{2.0f, 2.0f, 2.0f};

    glm::vec3 _up{ 0.0f, 0.0f, 1.0f};
    glm::vec3 _forward{ -1.0f, -1.0f, -1.0f};
    glm::vec3 _right{1.0f, 0.0f, 0.0f};

    float _yaw = -135.0f;
    float _pitch = -35.0f;

    float _fov = 45.0f;
    float _near_plane = 0.1f;
    float _far_plane = 100.0f;
};