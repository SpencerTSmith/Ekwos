#ifndef LOG_H
#define LOG_H

#include "core/common.h"

enum Log_Level {
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_DEBUG,
    LOG_INFO,
    LOG_MAX_NUM,
};

enum Exit_Code {
    EXT_SUCCESS,
    EXT_GLFW_INIT,
    EXT_VULKAN_SUPPORT,
    EXT_GLFW_WINDOW_CREATION,
    EXT_GLFW_EXTENSIONS,
    EXT_ARENA_ALLOCATION,
    EXT_ARENA_SIZE,
    EXT_VULKAN_INSTANCE,
    EXT_VULKAN_LAYERS,
    EXT_VULKAN_DEBUG_MESSENGER,
    EXT_VULKAN_NO_DEVICE,
    EXT_VULKAN_QUEUE_FAMILIES,
    EXT_VULKAN_LOGICAL_DEVICE,
    EXT_VULKAN_SURFACE,
    EXT_VULKAN_SWAP_CHAIN_INFO,
    EXT_VULKAN_SWAP_CHAIN_CREATE,
    EXT_VULKAN_SWAP_CHAIN_IMAGE_VIEW,
    EXT_VULKAN_SYNC_OBJECT,
    EXT_VULKAN_RENDER_PASS_CREATE,
    EXT_VULKAN_COMMAND_POOL,
    EXT_VULKAN_COMMAND_BUFFER,
    EXT_VULKAN_PIPELINE_LAYOUT,
    EXT_VULKAN_PIPELINE_CREATE,
    EXT_VULKAN_SHADER_MODULE,
    EXT_VULKAN_IMAGE_ACQUIRE,
    NUM_EXT
};

// TODO(spencer): rewrite using platform layer file i/o
// also look into custom asserts
void log_message(enum Log_Level level, const char *file, u64 line, const char *message, ...);

// The ## is specific to GCC... need to see about others
#define LOG_FATAL(message, ...) log_message(LOG_FATAL, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) log_message(LOG_ERROR, __FILE__, __LINE__, message, ##__VA_ARGS__)

// Can disable these messages at compile time
#ifdef DEBUG
#define LOG_WARN(message, ...) log_message(LOG_WARN, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define LOG_DEBUG(message, ...) log_message(LOG_DEBUG, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define LOG_WARN(message, ...) VOID_PROC
#define LOG_DEBUG(message, ...) VOID_PROC
#endif // DEBUG

#ifdef EXTRA_INFO
#define LOG_INFO(message, ...) log_message(LOG_INFO, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define LOG_INFO(message, ...) VOID_PROC
#endif // EXTRA_INFO

#endif // LOG_H
