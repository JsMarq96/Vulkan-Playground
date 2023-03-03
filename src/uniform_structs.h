#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include "glm/detail/qualifier.hpp"


struct sUniformBufferObject {
    glm::mat4x4 model;
    glm::mat4x4 view;
    glm::mat4x4 proj;
};