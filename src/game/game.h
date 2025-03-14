#ifndef GAME_H
#define GAME_H

#include "asset/asset_manager.h"

#include "core/window.h"

#include "game/args.h"
#include "game/camera.h"
#include "game/entity.h"

#include "render/render_context.h"

enum Game_Constants {
  GAME_DEFAULT_MAX_TICK = 240,
};

typedef struct Game Game;
struct Game {
  Window window;
  RND_Context render_context;
  ASS_Manager asset_manager;

  Arena persistent_arena;
  Arena frame_arena;

  Entity_Pool entity_pool;

  Camera camera;

  u64 target_frame_time_ns;
  f64 fps;
  f64 dt_s;
  u64 frame_count;
};

void game_init(Game *game, u32 argc, char **argv);
void game_free(Game *game);

#endif // GAME_H
