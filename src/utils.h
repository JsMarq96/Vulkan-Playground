#pragma once

#define assert_msg(condition, msg) if (!(condition)) {std::cout << msg << std::endl; assert(false);}
#define VK_OK(result, msg) if ((result) != VK_SUCCESS) { std::cout << "Vulkan validation error: " << result << " on " << msg << std::endl; assert(false);}
