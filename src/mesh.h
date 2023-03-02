#pragma once

#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <iostream>

#include <glm/glm.hpp>

#include "utils.h"

namespace Geometry {
    
    // =============================
    // GEOMETRY
    // =============================
    struct sVertex2D {
        glm::vec2 position;
        glm::vec3 color;
    };

    namespace Meshes {
        union SingleTriangle {
            sVertex2D vertices[3] = {
                { .position = {0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
                { .position = {0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f}},
                { .position = {-0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f}}
            };
            float raw_vertices[];
        };
    };


    // =============================
    // GEOMETRY DESCRIPTORS
    // =============================
    static VkVertexInputBindingDescription* get_2D_biding_description(uint32_t *count) {
        VkVertexInputBindingDescription* biding_descriptor = (VkVertexInputBindingDescription*) malloc(sizeof(VkVertexInputBindingDescription));
        biding_descriptor[0] = {
            .binding = 0, // Only in the first array
            .stride = sizeof(sVertex2D),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX // For using vertex OR isntancing
        };

        return biding_descriptor;
    }

    static VkVertexInputAttributeDescription* get_2D_attribute_description(uint32_t *count) {
        // Atribute config
        *count = 2;
        VkVertexInputAttributeDescription *attribute_description = (VkVertexInputAttributeDescription*) malloc(sizeof(VkVertexInputAttributeDescription) * 2);
        attribute_description[0] = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(sVertex2D, position)
        };
        attribute_description[1] = {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(sVertex2D, color)
        };

        return attribute_description;
    }
};
