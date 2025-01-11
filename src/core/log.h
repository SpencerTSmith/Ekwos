#ifndef LOG_H
#define LOG_H

#include "core/common.h"

typedef u64 Log_Level;
enum Log_Level {
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_DEBUG,
    LOG_INFO,
    LOG_MAX_NUM,
};

typedef u64 Exit_Code;
enum Exit_Code {
    EXT_SUCCESS,
    EXT_GLFW_INIT,
    EXT_VULKAN_SUPPORT,
    EXT_GLFW_WINDOW_CREATION,
    EXT_GLFW_EXTENSIONS,
    EXT_ARENA_ALLOCATION,
    EXT_ARENA_SIZE,
    EXT_VK_INSTANCE,
    EXT_VK_LAYERS,
    EXT_VK_DEBUG_MESSENGER,
    EXT_VK_NO_DEVICE,
    EXT_VK_QUEUE_FAMILIES,
    EXT_VK_LOGICAL_DEVICE,
    EXT_VK_SURFACE,
    EXT_VK_SWAP_CHAIN_INFO,
    EXT_VK_SWAP_CHAIN_CREATE,
    EXT_VK_SWAP_CHAIN_IMAGE_VIEW,
    EXT_VK_SYNC_OBJECT,
    EXT_VK_RENDER_PASS_CREATE,
    EXT_VK_COMMAND_POOL,
    EXT_VK_COMMAND_BUFFER,
    EXT_VK_PIPELINE_LAYOUT,
    EXT_VK_PIPELINE_CREATE,
    EXT_VK_SHADER_MODULE,
    EXT_VK_IMAGE_ACQUIRE,
    EXT_VK_DEPTH_FORMAT,
    EXT_VK_IMAGE_CREATE,
    EXT_VK_ALLOCATOR_INIT,
    EXT_VK_ALLOCATION,
    EXT_VK_MEMORY_BIND,
    EXT_VK_DEPTH_VIEW,
    EXT_COUNT
};

// TODO(spencer): rewrite using platform layer file i/o
// also look into custom asserts
void log_message(Log_Level level, const char *file, u64 line, const char *message, ...);

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
