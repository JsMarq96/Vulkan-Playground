#include "app.h"

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include "utils.h"

void sApp::create_image(const char* image_name, 
                        sTexture* texture) {
    // Load texture & store it to a satging buffer
    int text_width, text_height, text_channel_count;
    VkDeviceSize image_size;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    {
        // LOAD TEXTURE ==========================
        unsigned char* raw_pixels = stbi_load(image_name, 
                                            &text_width, 
                                            &text_height, 
                                            &text_channel_count, 
                                            STBI_rgb_alpha);

        assert_msg(raw_pixels != NULL, "Error loading image");

        image_size = text_width * text_height * 4; // 4 bits per pixel

        // COPY THE MEMORY TO A STAGINIG BUFFER ===================
        create_buffer(image_size, 
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                    &staging_buffer, 
                    &staging_memory);

        void *staging_memory_address;
        vkMapMemory(Vulkan.device, 
                    staging_memory, 
                    0, 
                    image_size, 
                    0, 
                    &staging_memory_address);
        
        memcpy(staging_memory_address, 
            raw_pixels, 
            image_size);

        vkUnmapMemory(Vulkan.device, 
                    staging_memory);

        
        //stbi_free((stbi_uc*)raw_pixels);
    }
    

    // Create the VkImage
    VkImage texture_image;
    {
        VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .extent = {
                .width = (uint32_t) text_width,
                .height = (uint32_t) text_height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL, // Changes for reading / writting??
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // transfer the memmory to, and set the sampler
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // Exclusive to the graphics queue
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Discard the contents on memory after the first Layout transition
        };

        VK_OK(vkCreateImage(Vulkan.device, 
                            &image_create_info, 
                            NULL, 
                            &texture_image),
              "Creating Vulkan Image");
    }
    

    // Reserve the memory for the VkImage
    VkDeviceMemory texture_image_memory;
    {
        // Get the requirements
        VkMemoryRequirements image_mem_requerements;
        vkGetImageMemoryRequirements(Vulkan.device, 
                                    texture_image, 
                                    &image_mem_requerements);

        VkMemoryAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = NULL,
            .allocationSize = image_mem_requerements.size,
            .memoryTypeIndex = find_memmory_type(Vulkan.physical_device, 
                                                 image_mem_requerements.memoryTypeBits, 
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };

        VK_OK(vkAllocateMemory(Vulkan.device, 
                               &alloc_info, 
                               NULL, 
                               &texture_image_memory), 
              "Allocating vk memeory for the iamge");
        
        // Bind the memmory and the image representation
        vkBindImageMemory(Vulkan.device, 
                          texture_image, 
                          texture_image_memory, 
                          0);
    }
   
    // Transition the image layout
    {
        // Set the layout to be written to
        transition_image_layout(texture_image, 
                                VK_FORMAT_R8G8B8A8_SRGB, 
                                VK_IMAGE_LAYOUT_UNDEFINED, 
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        // Write to
        copy_buffer_to_image(staging_buffer, 
                             texture_image, 
                             text_width, 
                             text_height, 
                             1);

        // Set the layout as a optimal layout for sampling
        transition_image_layout(texture_image, 
                                VK_FORMAT_R8G8B8A8_SRGB, 
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // Cleanup
    vkDestroyBuffer(Vulkan.device, staging_buffer, NULL);
    vkFreeMemory(Vulkan.device, staging_memory, NULL);

    // Return the texture
    texture->width = text_width;
    texture->height = text_height;
    texture->depth = 1;
    texture->texture_image = texture_image;
    texture->texture_image_memory = texture_image_memory;

}