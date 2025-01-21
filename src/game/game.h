#ifndef GAME_H
#define GAME_H

#include "core/window.h"

#include "game/camera.h"

#include "render/render_context.h"

typedef struct Game Game;
struct Game {
    Window window;
    RND_Context rctx;
    Camera camera;
    f64 fps;
    u64 frame_count;
    f64 dt;
};

void game_calc_fps_dt(Game *game);

void game_init(Game *game);

void game_free(Game *game);

#endif // GAME_H
