#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>

// Taken from Vulkan Samples, with couple modifications , the fatal version will exit with specified
// code for you
#define VK_CHECK_FATAL(x, exit_code, message, ...)                                                 \
    do {                                                                                           \
        VkResult check = x;                                                                        \
        if (check != VK_SUCCESS) {                                                                 \
            LOG_FATAL(message, ##__VA_ARGS__);                                                     \
            exit(exit_code);                                                                       \
        }                                                                                          \
    } while (0)

#define VK_CHECK_ERROR(x, message, ...)                                                            \
    do {                                                                                           \
        VkResult check = x;                                                                        \
        if (check != VK_SUCCESS) {                                                                 \
            LOG_ERROR(message, ##__VA_ARGS__);                                                     \
        }                                                                                          \
    } while (0)

#endif // RENDER_COMMON_H
