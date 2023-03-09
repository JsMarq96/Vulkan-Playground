#pragma once

#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <iostream>

#include "utils.h"

struct sTexture {
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    VkFormat format;

    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
    VkImageView texture_image_view;
    VkSampler sampler;

    VkDevice *device = NULL;
    VkPhysicalDevice *physical_device = NULL;

    void create_image_view() {
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = texture_image,
            .viewType = (depth > 1) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = { // Alter the was the RGBA channels are stored. Leave it as it comes
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Wich aspect of the iamge are used, in this case just the color
                .baseMipLevel = 0,
                .levelCount = 1, // No need for mipmaping (for now) on the swapchain
                .baseArrayLayer = 0, // multiplelayers could be usefull for multiple perspectives rendered at the same time
                .layerCount = 1
            }
        };

        VK_OK(vkCreateImageView(*device, 
                                &create_info, 
                                NULL, 
                                &texture_image_view),
             "Error creating image views of texture");
    }

    void create_sampler() {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(*physical_device, 
                                      &properties);

        VkSamplerCreateInfo sampler_create_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = NULL,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, // Wrap arround UVS using a repeat
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            .mipLodBias = 0.0f, // ??
            .anisotropyEnable  = VK_TRUE,
            .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE, // PCF on shadowmaps
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE, // The UVs are from 0,1
        };

        VK_OK(vkCreateSampler(*device, 
                              &sampler_create_info, 
                              NULL, 
                              &sampler), 
              "Texture sampler creationg");
    }


    void cleanup() {
        vkDestroySampler(*device, sampler, NULL);
        vkDestroyImageView(*device, texture_image_view, NULL);
        vkDestroyImage(*device, texture_image, NULL);
        vkFreeMemory(*device, texture_image_memory, NULL);
    }
};