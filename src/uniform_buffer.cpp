#include "app.h"
#include "uniform_structs.h"
#include <stdint.h>

void sApp::_create_descriptor_set_layout() {
    // ===============================
    // CREATE DESCRIPTION SET ========
    // ===============================
    {
        VkDescriptorSetLayoutBinding layout_bidings[2];
        layout_bidings[0] = { // UBO layout
            .binding = 0, // the position on the shader's memories
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1, // for uploading an array of UBOs
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // only for vertex shaders
            .pImmutableSamplers = NULL, // forimage samplers
        };

        layout_bidings[1] = { // sampler layout bidings
            .binding = 1, // the position on the shader's memories
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1, // for uploading an array of UBOs
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // only for vertex shaders
            .pImmutableSamplers = NULL, // forimage samplers
        };

        // Create layout
        VkDescriptorSetLayoutCreateInfo layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .bindingCount = 2,
            .pBindings = layout_bidings
        };

        VK_OK(vkCreateDescriptorSetLayout(Vulkan.device, 
                                        &layout_create_info, 
                                        NULL, 
                                        &Vulkan.descriptor_set_layout), 
            "Create descriptor set layout");
    }
}

void sApp::_create_uniform_buffers() {
    VkDeviceSize buffer_size = sizeof(sUniformBufferObject);

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        create_buffer(buffer_size, 
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                      &Vulkan.uniform_buffers[i], 
                      &Vulkan.uniform_buffers_memory[i]);
        
        vkMapMemory(Vulkan.device, 
                    Vulkan.uniform_buffers_memory[i], 
                    0, 
                    buffer_size, 
                    0, 
                    &Vulkan.uniform_buffers_mapped[i]);

        Vulkan.uniform_buffer_count++;
    }
}

void sApp::_create_descriptor_pool_and_set() {
    // ===============================
    // CREATE DESCRIPTION POOL =======
    // ===============================
    {
        VkDescriptorPoolSize pool_sizes[2];
        pool_sizes[0] = { // Ubo descriptor pool size
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        };

        pool_sizes[1] = { // Samplers descriptor pool size
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        };

        VkDescriptorPoolCreateInfo pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .maxSets = MAX_FRAMES_IN_FLIGHT,
            .poolSizeCount = 2,
            .pPoolSizes = pool_sizes,
        };

        VK_OK(vkCreateDescriptorPool(Vulkan.device, 
                                    &pool_create_info, 
                                    NULL, 
                                    &Vulkan.descriptor_pool), 
            "Create descriptor pool");
    }
    
    // ===============================
    // CREATE DESCRIPTION SET ========
    // ===============================
    {
        VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            layouts[i] = Vulkan.descriptor_set_layout;
        }
        
        VkDescriptorSetAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = Vulkan.descriptor_pool,
            .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
            .pSetLayouts = layouts
        }; 

        VK_OK(vkAllocateDescriptorSets(Vulkan.device, 
                                       &alloc_info, 
                                       Vulkan.descriptor_sets), 
              "Descritor set allocations");
        
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo buffer_info = {
                .buffer = Vulkan.uniform_buffers[i],
                .offset = 0,
                .range = sizeof(sUniformBufferObject)
            };

            VkDescriptorImageInfo image_info = {
                .sampler = texture.sampler,
                .imageView = texture.texture_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };

            VkWriteDescriptorSet descriptor_set_write[2];
            descriptor_set_write[0] = { // UBO descriptor write set
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = Vulkan.descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0, // Not an array, so first element
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = NULL,  // For image data
                .pBufferInfo = &buffer_info,
                .pTexelBufferView = NULL, // For view buffers
            };

            descriptor_set_write[1] = { // Texture sampler descriptor write set
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = Vulkan.descriptor_sets[i],
                .dstBinding = 1,
                .dstArrayElement = 0, // Not an array, so first element
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &image_info,  // For image data
                .pBufferInfo = NULL,
                .pTexelBufferView = NULL, // For view buffers
            };

            vkUpdateDescriptorSets(Vulkan.device, 
                                   2, 
                                   descriptor_set_write, 
                                   0, // No need for copying descriptors
                                   NULL);
        }
    }
}