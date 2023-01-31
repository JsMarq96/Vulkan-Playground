#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME   "Vulkan test"

#define assert_msg(condition, msg) if (!(condition)) {std::cout << msg << std::endl; assert(false);}

struct sApp {
    GLFWwindow *window = NULL;


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

    void _init_vulkan() {
        // TODO
    }

    void _clean_up() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void _main_loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
};