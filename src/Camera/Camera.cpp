#include <OrcaEngine/Camera/Camera.hpp>

Camera::Camera() {}

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

void Camera::UpdateDirectionVectors()
{

}
