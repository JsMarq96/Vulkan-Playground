#pragma once

#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <iostream>

#include "utils.h"

inline void create_shader_module(const VkDevice &device, const char* shader_dir, VkShaderModule *shader_module) {
    FILE *file = NULL;

    file = fopen(shader_dir, "rb");

    assert_msg(file != NULL, "Error opening shader file");

    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *raw_shader = (char*) malloc(sizeof(char) * size + 1);

    fread(raw_shader, size, 1, file);

    fclose(file);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .codeSize = sizeof(char) * size,
        .pCode = (uint32_t*) raw_shader
    };

    VK_OK(vkCreateShaderModule(device, 
                               &create_info, 
                               NULL,
                               shader_module),
          "Error creating shader module");

    free(raw_shader);
}


struct sShader {
    

};