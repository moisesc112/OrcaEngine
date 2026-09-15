#include <OrcaEngine/Camera/Camera.hpp>

Camera::Camera() 
{
    UpdateDirectionVectors();
}

Camera::~Camera() {}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(_position, _position + _forward, _up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspect_ratio)
{
    glm::mat4 projection_matrix = glm::perspective(glm::radians(_fov), aspect_ratio, _near_plane, _far_plane);
    projection_matrix[1][1] *= -1.0f;

    return projection_matrix;
}

void Camera::SetPosition(glm::vec3& position)
{
    _position = position;
}

void Camera::SetRotation(float yaw, float pitch)
{
    _yaw = yaw;
    _pitch = glm::clamp(pitch, -89.0f, 89.0f);

    UpdateDirectionVectors();
}

void Camera::MoveForward(float amount)
{
    _position += _forward * amount;
}

void Camera::MoveRight(float amount)
{
    _position += _right * amount;
}

void Camera::MoveUp(float amount)
{
    _position += _up * amount;
}

void Camera::AddYaw(float amount)
{
    _yaw += amount;
    UpdateDirectionVectors();
}

void Camera::AddPitch(float amount)
{
    _pitch = glm::clamp(_pitch + amount, -89.0f, 89.0f);
    UpdateDirectionVectors();
}

void Camera::UpdateDirectionVectors()
{
    glm::vec3 forward{};

    forward.x = std::cos(glm::radians(_pitch)) * std::cos(glm::radians(_yaw));
    forward.y = std::cos(glm::radians(_pitch)) * std::sin(glm::radians(_yaw));
    forward.z = std::sin(glm::radians(_pitch));

    _forward = glm::normalize(forward);

    glm::vec3 world_up{ 0.0f, 0.0f, 1.0f};

    _right = glm::normalize(glm::cross(_forward, world_up));
    _up = glm::normalize(glm::cross(_right, _forward));
}

