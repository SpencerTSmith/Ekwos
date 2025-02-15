#ifndef ARGS_H
#define ARGS_H

#include "core/common.h"

typedef struct Config Config;
struct Config {
  u32 window_width, window_height;
};

typedef i32 Argument;
enum Argument {
  ARG_INVALID = -1,
  ARG_WINDOW_WIDTH,
  ARG_WINDOW_HEIGHT,
  ARG_MAX,
};

Argument arg_from_string(char *arg);
Config arg_parse(u32 argc, char **argv);

#endif // ARGS_H
