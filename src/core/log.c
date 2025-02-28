#include "core/log.h"

#include <stdarg.h>
#include <stdio.h>

translation_local const char *level_strings[LOG_MAX_NUM] = {"FATAL", "ERROR", "WARNING", "DEBUG"};

void log_message(Log_Level level, const char *file, u64 line, const char *message, ...) {
  if (level <= LOG_WARN) {
    fprintf(stderr, "[Ekwos %s]: (%s:%lu) ", level_strings[level], file, line);
  } else {
    fprintf(stderr, "[Ekwos %s]: ", level_strings[level]);
  }

  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);

  fprintf(stderr, "\n");
}
