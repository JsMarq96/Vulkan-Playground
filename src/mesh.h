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
        glm::vec2 text_coord;
    };

    namespace Meshes {
        namespace SingleTriangle {
            const uint32_t indices_count = 3;
            const uint32_t indices[3] = { 0,1,2 };
            const uint32_t vertices_count = 3;
            const sVertex2D vertices[3] = {
                    { .position = {0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f} },
                    { .position = {0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f} },
                    { .position = {-0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f} }
            };
        };

        namespace Quad {
            const uint32_t indices_count = 6;
            const uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
            const uint32_t vertices_count = 4;
            const sVertex2D vertices[4] = {
                    { .position = {-0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}, .text_coord = {1.0f, 0.0f} },
                    { .position = {0.5f, -0.5f}, .color = {1.0f, 1.0f, 0.0f}, .text_coord = {0.0f, 0.0f} },
                    { .position = {0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f}, .text_coord = {0.0f, 1.0f} },
                    { .position = {-0.5f, 0.5f}, .color = {0.0f, 0.0f, 0.0f}, .text_coord = {1.0f, 1.0f} }
            };
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
        *count = 3;
        VkVertexInputAttributeDescription *attribute_description = (VkVertexInputAttributeDescription*) malloc(sizeof(VkVertexInputAttributeDescription) * 3);
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
        attribute_description[2] = {
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(sVertex2D, text_coord)
        };

        return attribute_description;
    }
};
