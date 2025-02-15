#include "args.h"

#include "core/log.h"
#include "window.h"

#include <stdlib.h>
#include <string.h>

Argument arg_from_string(char *arg) {
  if (strcmp(arg, "--window-width") == 0 || strcmp(arg, "-w") == 0)
    return ARG_WINDOW_WIDTH;
  if (strcmp(arg, "--window-height") == 0 || strcmp(arg, "-h") == 0)
    return ARG_WINDOW_HEIGHT;

  return ARG_INVALID;
}

Config arg_parse(u32 argc, char **argv) {
  // Defaults
  Config config = {
      .window_width = WINDOW_DEFAULT_WIDTH,
      .window_height = WINDOW_DEFAULT_HEIGHT,
  };

  if (argc == 1) {
    return config;
  }

  for (u32 i = 1; i < argc; i++) {
    Argument arg = arg_from_string(argv[i]);
    switch (arg) {
    case ARG_WINDOW_WIDTH:
      if (i + 1 >= argc) {
        LOG_ERROR("Please include a second argument for --window-width or -w");
        continue;
      }

      // HACK(ss): Eventually think about replacing atoi, not always safe
      config.window_width = atoi(argv[i + 1]);
      LOG_INFO("CLI: window width set to %lu", config.window_width);
      break;
    case ARG_WINDOW_HEIGHT:
      if (i + 1 >= argc) {
        LOG_ERROR("Please include a second argument for --window-height or -h");
        continue;
      }

      // HACK(ss): Eventually think about replacing atoi, not always safe
      config.window_height = atoi(argv[i + 1]);
      LOG_INFO("CLI: window height set to %lu", config.window_height);
      break;
    }
  }

  return config;
}
