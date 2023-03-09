#include "app.h"


uint32_t sApp::find_memmory_type(const VkPhysicalDevice &phys_device,
                           const  uint32_t type_filter, 
                           const VkMemoryPropertyFlags &properties) {
    // Get all the hardware s memmory props
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(phys_device, 
                                        &mem_properties);
    
    // Check the type of the memmory and also the properties (if its writable from the CPU for example)
    for(uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    assert_msg(false, "No usable memmory type");
    return 0; // TODO: not very good
}


void sApp::create_buffer(const VkDeviceSize &size, 
                         const VkBufferUsageFlags usage,
                         const VkMemoryPropertyFlags memmory_properties, 
                         VkBuffer *buffer, 
                         VkDeviceMemory *buffer_memory) {
    // Create Buffer
    VkBufferCreateInfo vertex_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VK_OK(vkCreateBuffer(Vulkan.device, 
                        &vertex_buffer_info,
                        NULL, 
                        buffer), 
         "Creating buffer");
    
    // Query memmory requirements
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(Vulkan.device,
                                  *buffer,
                                  &memory_requirements);
    
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memmory_type(Vulkan.physical_device, 
                                             memory_requirements.memoryTypeBits, 
                                             memmory_properties) // HOST coherent to flush the mapped area before writing
    };

    VK_OK(vkAllocateMemory(Vulkan.device, 
                          &alloc_info, 
                          NULL, 
                          buffer_memory), 
          "Allocating buffer memory");
    
    // Associate the memory with the buffer description
    vkBindBufferMemory(Vulkan.device, 
                       *buffer, 
                       *buffer_memory, 
                       0); // Offset
}

void sApp::copy_buffer(const VkBuffer &src_buffer,
                       const VkBuffer dst_buffer,
                       const VkDeviceSize size) {
    VkCommandBuffer command_buffer = being_single_time_commands();

    VkBufferCopy copy_region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, 
                    src_buffer,
                    dst_buffer,
                    1, // only one region to copy
                    &copy_region);

    end_single_time_commands(command_buffer);
}


VkCommandBuffer sApp::being_single_time_commands() {
    // Create & start the comand buffer
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = Vulkan.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // high propity
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(Vulkan.device, 
                             &alloc_info, 
                             &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    // Record just the copy operation on the command buffer
    vkBeginCommandBuffer(command_buffer, 
                         &begin_info);

    return command_buffer;
}
void sApp::end_single_time_commands(const VkCommandBuffer &command_buffer) {
    vkEndCommandBuffer(command_buffer);

    // Submit & launch the command buffer
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(Vulkan.graphics_queue, 
                  1,
                  &submit_info, 
                  VK_NULL_HANDLE);

    vkQueueWaitIdle(Vulkan.graphics_queue); // Note a fence could allow multiple transfers at the same time!

    vkFreeCommandBuffers(Vulkan.device, 
                         Vulkan.command_pool, 
                         1, 
                         &command_buffer);
}


// TEXTURE HELPER

void sApp::transition_image_layout(const VkImage &image, 
                                   const VkFormat &format, 
                                   const VkImageLayout &old_layout, 
                                   const VkImageLayout &new_layout) {
    VkCommandBuffer command_buffer = being_single_time_commands();

    VkAccessFlags source_access_mask;
    VkAccessFlags dest_access_mask;
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags dest_stage;

    // Change the pipeline state of the image for each transitions
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && 
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        source_access_mask = 0;
        dest_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        source_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dest_access_mask = VK_ACCESS_SHADER_READ_BIT;
        
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        assert_msg(false, "Invalid iamge transition layout");
    }

    // use a memmory barrier to wait for the memomry operations to be finalized
    // before reading and useing teh memory
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = source_access_mask,
        .dstAccessMask = dest_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Only get the color data
            .baseMipLevel = 0, // For mipmapping
            .levelCount = 1,
            .baseArrayLayer = 0, // for layered / 3d textures
            .layerCount = 1
        }
    };

    vkCmdPipelineBarrier(command_buffer, 
                         source_stage, dest_stage,
                         0,
                         0, NULL, 
                         0, NULL,
                         1, &barrier);

    end_single_time_commands(command_buffer);
}


void sApp::copy_buffer_to_image(const VkBuffer &buffer, 
                                const VkImage &image, 
                                const uint32_t width, 
                                const uint32_t height,
                                const uint32_t depth) {

    VkCommandBuffer command_buffer = being_single_time_commands();

    VkBufferImageCopy copy_region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {
            .x = 0, .y = 0, .z = 0
        },
        .imageExtent = {
            .width = width,
            .height = height,
            .depth = depth
        }
    };

    vkCmdCopyBufferToImage(command_buffer, 
                           buffer, 
                           image, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // The layout is the optimal for copying pixels
                           1, 
                           &copy_region);

    end_single_time_commands(command_buffer);

}