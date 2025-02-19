#ifndef GAME_H
#define GAME_H

#include "asset/asset_manager.h"

#include "core/args.h"
#include "core/window.h"

#include "game/camera.h"

#include "game/entity.h"
#include "render/render_context.h"

typedef struct Game Game;
struct Game {
  Config config;

  Window window;
  RND_Context render_context;
  ASS_Manager asset_manager;

  Entity_Pool entity_pool;

  Arena persistent_arena;

  Camera camera;

  f64 target_frame_time_ms;
  f64 fps;
  f64 dt;
  u64 frame_count;
};

void game_init(Game *game, u32 argc, char **argv);
void game_free(Game *game);

#endif // GAME_H
