#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "app.h"


int main() {
    sApp app = {};

    app.run();

    return 0;
}