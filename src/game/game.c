#include "game/game.h"

void game_init(Game *game, u32 argc, char **argv) {
  Config config = arg_parse(argc, argv);

  // Initialize the game's window
  window_init(&game->window, "Ekwos", config.window_width, config.window_height);

  // Initialize the game's render context
  rnd_context_init(&game->render_context, &game->window);

  // Create our persistent arena (Long term state, probably for the whole lifetime of the game)
  game->persistent_arena = arena_make(1024 * 256, ARENA_FLAG_DEFAULTS);
  game->frame_arena = arena_make(1024 * 256, ARENA_FLAG_DEFAULTS);

  game->entity_pool = entity_pool_make(ENTITY_MAX_NUM);

  // Initialize the game's asset manager
  ass_manager_init(&game->persistent_arena, &game->asset_manager);

  // Default Camera Settings
  game->camera = (Camera){
      .sensitivity = .2f,
      .move_speed = 10.f,
  };

  // camera_set_direction(&game->camera, vec3(0.f, 0.f, 0.f), vec3(0.0f, 0.f, -1.f),
  //                      vec3(0.f, 1.f, 0.f));

  game->target_frame_time_ns = (1e9 / config.fps_limit);
}

void game_free(Game *game) {
  ass_manager_free(&game->asset_manager, &game->render_context);
  entity_pool_free(&game->entity_pool);
  arena_free(&game->frame_arena);
  arena_free(&game->persistent_arena);
  rnd_context_free(&game->render_context);
  window_free(&game->window);
  ZERO_STRUCT(game);
}
