#include "game/game.h"

// Should this be here?
void game_calc_fps_dt(Game *game) {}

void game_init(Game *game, u32 argc, char **argv) {
  Config config = arg_parse(argc, argv);

  // Initialize the game's window
  window_init(&game->window, "Ekwos", config.window_width, config.window_height);

  // Initialize the game's render context
  rnd_context_init(&game->render_context, &game->window);

  game->entity_pool = entity_pool_create(ENTITY_MAX_NUM);

  // Create our persistent arena (Long term state, probably for the whole lifetime of the game)
  game->persistent_arena = arena_create(1024 * 256, ARENA_FLAG_DEFAULTS);

  // Initialize the game's asset manager
  ass_manager_init(&game->persistent_arena, &game->asset_manager);

  // Default Camera Settings
  camera_set_perspective(&game->camera, RADIANS(90.f), (f32)game->window.w / game->window.h, .1f,
                         10.f);
  camera_set_direction(&game->camera, vec3(0.f, 0.f, 0.f), vec3(0.0f, 0.f, -1.f),
                       vec3(0.f, 1.f, 0.f));
}

void game_free(Game *game) {
  entity_pool_free(&game->entity_pool);
  rnd_context_free(&game->render_context);
  window_free(&game->window);
  arena_free(&game->persistent_arena);
  ZERO_STRUCT(game);
}
