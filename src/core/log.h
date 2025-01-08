#ifndef LOG_H
#define LOG_H

#include "core/common.h"

#include <assert.h>
#include <stdarg.h>

enum Log_Level {
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARN,
    LOG_DEBUG,
    LOG_MAX_NUM,
};

// TODO(spencer): rewrite using platform layer file i/o
void log_message(enum Log_Level level, const char *file, u64 line, const char *message, ...);

#define LOG_FATAL(message, ...) log_message(LOG_FATAL, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) log_message(LOG_ERROR, __FILE__, __LINE__, message, ##__VA_ARGS__)

// Can disable these messages at compile time
#ifdef DEBUG
#define LOG_WARN(message, ...) log_message(LOG_WARN, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define LOG_DEBUG(message, ...) log_message(LOG_DEBUG, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define LOG_WARN(message, ...)
#define LOG_DEBUG(message, ...)
#endif

#endif // LOG_H
