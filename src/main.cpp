#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME   "Vulkan test"

void key_callback(GLFWwindow *wind, 
                  int key, 
                  int scancode, 
                  int action, 
                  int mods) {
	// ESC to close the game
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(wind, 
                                 GL_TRUE);
	}
}

int main() {
    if (!glfwInit()) {
        std::cout << "Error cound not create window" << std::endl;
        return 0;
    }

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, 
                                         WINDOW_HEIGHT, 
                                         WINDOW_NAME, 
                                         NULL, 
                                         NULL);

    glfwSetKeyCallback(window, key_callback);

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    std::cout << extension_count << " extensions" << std::endl;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}