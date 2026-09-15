#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent {
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };

    glm::vec3 GetForwardDirection() 
    {
        glm::mat4 rotation_matrix { 1.0f };

        rotation_matrix = glm::rotate(rotation_matrix, 
                                      glm::radians(rotation.z),
                                      glm::vec3(0.0f, 0.0f, 1.0f));

        rotation_matrix = glm::rotate(rotation_matrix, 
                                      glm::radians(rotation.y),
                                      glm::vec3(0.0f, 1.0f, 0.0f));

        rotation_matrix = glm::rotate(rotation_matrix, 
                                      glm::radians(rotation.x),
                                      glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 local_forward = { 0.0f, 0.0f, -1.0f };

        return glm::normalize(glm::vec3(rotation_matrix * glm::vec4(local_forward, 0.0f)));
    }

};