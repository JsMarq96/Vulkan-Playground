#pragma once

#include <stdint.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME   "Vulkan test"
#define ENGINE_NAME   "No engine"

#define assert_msg(condition, msg) if (!(condition)) {std::cout << msg << std::endl; assert(false);}
#define VK_OK(result, msg) if ((result) != VK_SUCCESS) { std::cout << "Vulkan validation error: " << result << " on " << msg << std::endl; assert(false);}

struct sQueueFamilies {
    uint32_t graphics_family_id;
    bool has_found_graphics_family = false;
};

// https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Window_surface
struct sApp {
    GLFWwindow *window = NULL;

    // Vulkan data
    struct {
        VkInstance instance;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        sQueueFamilies queues;
        VkDevice device; // logical device
        VkQueue  graphics_queue;

#ifndef NDEBUG
        // Validation layers
        const char* required_validation_layers[2] = {
            "VK_LAYER_KHRONOS_validation",
        };
        uint32_t required_validation_layer_count = 1;

        const char* required_extensions[10] = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };
        uint32_t required_extension_count = 1;

        // Debug messaegs
        VkDebugUtilsMessengerEXT debug_messenger;
#endif
    } Vulkan;

    void run() {
        _init_window();
        _init_vulkan();
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

    void _clean_up() {
        vkDestroyDevice(Vulkan.device, NULL);
        // TODO destroy the Utils messener: add it to the Vulkna struct
        vkDestroyInstance(Vulkan.instance, NULL);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void _main_loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
};