#include "args.h"

#include "core/log.h"
#include "window.h"

#include <stdlib.h>
#include <string.h>

// Is this over engineering?
static const char *arg_strings[] = {"--window-width", "--window-height"};
static const char *arg_short_strings[] = {"-w", "-h"};

Argument arg_from_string(char *arg) {
  if (strcmp(arg, arg_strings[ARG_WINDOW_WIDTH]) == 0 ||
      strcmp(arg, arg_short_strings[ARG_WINDOW_WIDTH]) == 0)
    return ARG_WINDOW_WIDTH;
  if (strcmp(arg, arg_strings[ARG_WINDOW_HEIGHT]) == 0 ||
      strcmp(arg, arg_short_strings[ARG_WINDOW_HEIGHT]) == 0)
    return ARG_WINDOW_HEIGHT;

  return ARG_INVALID;
}

Config arg_parse(u32 argc, char **argv) {
  // Defaults
  Config config = {
      .window_width = WINDOW_DEFAULT_WIDTH,
      .window_height = WINDOW_DEFAULT_HEIGHT,
  };

  if (argc == 1)
    return config;

  for (u32 i = 1; i < argc; i++) {
    Argument arg = arg_from_string(argv[i]);
    switch (arg) {
    case ARG_WINDOW_WIDTH:
      if (i + 1 >= argc) {
        LOG_ERROR("Please include a value for %s or %s", arg_strings[ARG_WINDOW_WIDTH],
                  arg_short_strings[ARG_WINDOW_WIDTH]);
        continue;
      }

      // HACK(ss): Eventually think about replacing atoi, not always safe
      config.window_width = atoi(argv[i + 1]);
      LOG_INFO("Window width set to %lu", config.window_width);
      i++; // We can skip the next argument

      break;
    case ARG_WINDOW_HEIGHT:
      if (i + 1 >= argc) {
        LOG_ERROR("Please include a value for %s or %s", arg_strings[ARG_WINDOW_HEIGHT],
                  arg_short_strings[ARG_WINDOW_HEIGHT]);
        continue;
      }

      // HACK(ss): Eventually think about replacing atoi, not always safe
      config.window_height = atoi(argv[i + 1]);
      LOG_INFO("Window height set to %lu", config.window_height);
      i++; // We can skip the next argument

      break;
    case ARG_INVALID:
      LOG_ERROR("Unkown argument: \"%s\"", argv[i]);
    }
  }

  return config;
}
