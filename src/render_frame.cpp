#include "app.h"

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "uniform_structs.h"


void sApp::_render_frame() {
    // Wait for the prev frame is finished
    vkWaitForFences(Vulkan.device,
                    1,
                    &Vulkan.in_flight_fence[Vulkan.current_frame],
                    VK_TRUE,
                    UINT64_MAX);
    vkResetFences(Vulkan.device,
                  1,
                  &Vulkan.in_flight_fence[Vulkan.current_frame]);
    // Adquire swapchian image
    vkAcquireNextImageKHR(Vulkan.device,
                          Vulkan.swapchain,
                          UINT64_MAX,
                          Vulkan.image_available_semaphore[Vulkan.current_frame],
                          VK_NULL_HANDLE,
                          &Vulkan.swapchain_images_index);

    // Update the uniform buffers
    {
        // Eeegghhhhhjjjjjj
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        sUniformBufferObject ubo = {
            .model = glm::rotate(glm::mat4(1.0f), 
                                 time * glm::radians(90.0f), 
                                 glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), 
                                glm::vec3(0.0f, 0.0f, 0.0f), 
                                glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f), 
                                     Vulkan.swapchain_info.swapchain_extent.width / (float) Vulkan.swapchain_info.swapchain_extent.height, 
                                     0.1f, 
                                     10.0f)
        };
        // Invert the Y coordnate, since glm is  made with opengl in mind
        ubo.proj[1][1] *= -1.0f;

        // Copy to the mapped memmory 
        memcpy(Vulkan.uniform_buffers_mapped[Vulkan.current_frame], 
               &ubo, 
               sizeof(ubo));
    }

    // Add teh command buffer
    vkResetCommandBuffer(Vulkan.command_buffers[Vulkan.current_frame],
                         0);

    record_command_buffer(Vulkan.command_buffers[Vulkan.current_frame],
                          Vulkan.render_pass,
                          Vulkan.swapchain_images_index);

    // Submit the command buffer
    VkPipelineStageFlags wait_stagers[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan.image_available_semaphore[Vulkan.current_frame],
        .pWaitDstStageMask = wait_stagers,
        .commandBufferCount = 1,
        .pCommandBuffers = &Vulkan.command_buffers[Vulkan.current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &Vulkan.render_finished_semaphore[Vulkan.current_frame]
    };

    VK_OK(vkQueueSubmit(Vulkan.graphics_queue, 
                        1, 
                        &submit_info, 
                        Vulkan.in_flight_fence[Vulkan.current_frame]), 
          "Submit Queue frame");

    // Presentation
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan.render_finished_semaphore[Vulkan.current_frame],
        .swapchainCount = 1,
        .pSwapchains = &Vulkan.swapchain,
        .pImageIndices = &Vulkan.swapchain_images_index,
        .pResults = NULL
    };

    VK_OK(vkQueuePresentKHR(Vulkan.graphics_queue, 
                            &present_info),
          "Presenting frame");

    Vulkan.current_frame = (Vulkan.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}