#pragma once

#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_EXPOSE_NATIVE_X11
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <GLFW/glfw3native.h>

#include <cassert>
#include <iostream>

#include "utils.h"
#include "textures.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME   "Vulkan test"
#define ENGINE_NAME   "No engine"

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_UNIFORM_BUFFERS 5 * MAX_FRAMES_IN_FLIGHT
#define MAX_DESCRIPTOR_SETS 5 * MAX_FRAMES_IN_FLIGHT

struct sQueueFamilies {
    uint32_t graphics_family_id;
    bool has_found_graphics_family = false;
    uint32_t presenting_family_id;
    bool has_found_presenting_familiy = false;
};

struct sSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilites;
    VkSurfaceFormatKHR  *formats = NULL;
    uint32_t format_count = 0;
    VkSurfaceFormatKHR selected_format;

    VkExtent2D swapchain_extent;

    VkPresentModeKHR  *present_modes = NULL;
    uint32_t present_modes_count = 0;
    VkPresentModeKHR  selected_present_mode;

    void clean() {
        free(formats);
        free(present_modes);
    }
};

struct sApp {
    GLFWwindow *window = NULL;

    sTexture texture;

    // Vulkan data
    struct {
        VkInstance instance;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        sQueueFamilies queues;
        VkDevice device; // logical device

        VkQueue  graphics_queue;
        VkQueue  present_queue;
        VkSurfaceKHR surface;

        sSwapchainSupportInfo swapchain_info;
        VkSwapchainKHR swapchain;

        VkPipeline graphics_pipeline;
        VkRenderPass render_pass;

        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet  descriptor_sets[MAX_DESCRIPTOR_SETS];
        uint32_t descriptor_sets_count = 0;

        VkPipelineLayout pipeline_layout;

        VkFramebuffer *framebuffers = NULL;
        uint32_t framebuffers_count = 0;

        VkImage *swapchain_images;
        VkImageView *swapchain_image_views;
        uint32_t swapchain_images_count = 0;
        uint32_t swapchain_images_index = 0;

        VkCommandPool command_pool;
        VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];

        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memmory;
        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;

        uint32_t    current_frame = 0;
        VkSemaphore image_available_semaphore[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore render_finished_semaphore[MAX_FRAMES_IN_FLIGHT];
        VkFence in_flight_fence[MAX_FRAMES_IN_FLIGHT]; // ???

        // Uniform buffers
        VkBuffer uniform_buffers[MAX_UNIFORM_BUFFERS];
        VkDeviceMemory uniform_buffers_memory[MAX_UNIFORM_BUFFERS];
        void*   uniform_buffers_mapped[MAX_UNIFORM_BUFFERS];
        uint32_t uniform_buffer_count = 0;

        // Validation layers
        const char* required_validation_layers[2] = {
            "VK_LAYER_KHRONOS_validation",
        };
        uint32_t required_validation_layer_count = 1;

        // Instance extentions
        const char* required_extensions[10] = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };
        uint32_t required_extension_count = 1;

        // Device extensions
        const char* required_device_extensions[10] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        uint32_t required_device_extension_count = 1;

        // Debug messaegs
        VkDebugUtilsMessengerEXT debug_messenger;
    } Vulkan;

    void run() {
        _init_window();
        _init_vulkan();
        _create_descriptor_set_layout();
        _create_graphics_pipeline();
        _create_framebuffers();
        _create_command_buffers();

        create_image("resources/bop.jpg", 
                     &texture);

        texture.create_image_view();
        texture.create_sampler();

        _create_sync_objects();
        _main_loop();
        _clean_up();
    };

    // EVENT FUNCTIONS
    
    // LIFE CYCLE FUNCTIONS
    void _init_window() {
        assert_msg(glfwInit(),"Could init GLFW");

        glfwWindowHint(GLFW_CLIENT_API, 
                       GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, 
                       GLFW_FALSE);

        window = glfwCreateWindow(WINDOW_WIDTH, 
                                 WINDOW_HEIGHT, 
                                 WINDOW_NAME, 
                                 NULL, 
                                 NULL);

        assert_msg(window != NULL, "Could not create window");

        glfwSetKeyCallback(window, 
                           [](GLFWwindow *window, 
                              int key, 
                              int scancode, 
                              int action, 
                              int mods) {
                                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                                    glfwSetWindowShouldClose(window, 
                                                             GL_TRUE);
                                }
                            });
    }

    void _init_vulkan();

    void _create_descriptor_set_layout();

    void _create_uniform_buffers();

    void _create_descriptor_pool_and_set();

    void _create_graphics_pipeline();

    void _create_framebuffers();

    void _create_command_buffers();

    void _create_vertex_buffer();

    void _create_index_buffer();

    void _create_sync_objects();

    void _render_frame();


    // TODO: clean shaders
    void _clean_up() {
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(Vulkan.device, Vulkan.image_available_semaphore[i], NULL);
            vkDestroySemaphore(Vulkan.device, Vulkan.render_finished_semaphore[i], NULL);
            vkDestroyFence(Vulkan.device, Vulkan.in_flight_fence[i], NULL);
        }
        vkDestroyCommandPool(Vulkan.device, Vulkan.command_pool, NULL);

        for(uint32_t i = 0; i < Vulkan.framebuffers_count; i++) {
            vkDestroyFramebuffer(Vulkan.device, Vulkan.framebuffers[i], NULL);
        }
        free(Vulkan.framebuffers);

        vkDestroyPipeline(Vulkan.device, Vulkan.graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(Vulkan.device, Vulkan.pipeline_layout, nullptr);

        vkDestroyRenderPass(Vulkan.device, Vulkan.render_pass, NULL);

        Vulkan.swapchain_info.clean();

        for(uint32_t i = 0; i < Vulkan.uniform_buffer_count; i++) {
            vkDestroyBuffer(Vulkan.device, Vulkan.uniform_buffers[i], NULL);
            vkFreeMemory(Vulkan.device, Vulkan.uniform_buffers_memory[i], NULL);
        }

        vkDestroyDescriptorPool(Vulkan.device, Vulkan.descriptor_pool, NULL);

        vkDestroyDescriptorSetLayout(Vulkan.device, Vulkan.descriptor_set_layout, NULL);

        vkDestroyBuffer(Vulkan.device, Vulkan.vertex_buffer, NULL);
        vkFreeMemory(Vulkan.device, Vulkan.vertex_buffer_memmory, NULL);

        vkDestroyBuffer(Vulkan.device, Vulkan.index_buffer, NULL);
        vkFreeMemory(Vulkan.device, Vulkan.index_buffer_memory, NULL);

        for(uint32_t i = 0; i < Vulkan.swapchain_images_count; i++) {
            vkDestroyImageView(Vulkan.device, Vulkan.swapchain_image_views[i], NULL);
        }

        texture.cleanup();

        vkDestroyBuffer(Vulkan.device, Vulkan.vertex_buffer, NULL);
        vkDestroySwapchainKHR(Vulkan.device, Vulkan.swapchain, NULL);
        vkDestroyDevice(Vulkan.device, NULL);
        vkDestroySurfaceKHR(Vulkan.instance, Vulkan.surface, NULL);
        // TODO destroy the Utils messener: add it to the Vulkna struct
        free(Vulkan.swapchain_images);
        vkDestroyInstance(Vulkan.instance, NULL);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    // ===============================
    // HELPER FUNCTIONS
    // ===============================
    void create_buffer(const VkDeviceSize &size, 
                       const VkBufferUsageFlags usage,
                       const VkMemoryPropertyFlags memmory_properties, 
                       VkBuffer *buffer, 
                       VkDeviceMemory *buffer_memory);
    void copy_buffer(const VkBuffer &src_buffer, const VkBuffer dst_buffer, const VkDeviceSize size);

    void record_command_buffer(const VkCommandBuffer &command_buffer,
                           const VkRenderPass &render_pass,
                           const uint32_t image_index);

    void create_image(const char* image_name, sTexture *texture);

    uint32_t find_memmory_type(const VkPhysicalDevice &phys_device,
                           const  uint32_t type_filter, 
                           const VkMemoryPropertyFlags &properties);

    VkCommandBuffer being_single_time_commands();
    void end_single_time_commands(const VkCommandBuffer &command_buffer);

    void copy_buffer_to_image(const VkBuffer &buffer, const VkImage &image, const uint32_t width, const uint32_t height,  const uint32_t depth);
    void transition_image_layout(const VkImage &image, const VkFormat &format, const VkImageLayout &old_layout, const VkImageLayout &new_layout);

    void _main_loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            _render_frame();
        }

        vkDeviceWaitIdle(Vulkan.device);
    }
};
