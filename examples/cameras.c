/* clang-format off */

/*
  FlyGPU
  Copyright (C) 2025-2026 Domán Zana

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/macros.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>        /* IWYU pragma: keep */
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define MOVE_SPEED 0.01F
#define ROLL_SPEED (MOVE_SPEED / 2.0F)

static const char *ALBEDOS[3] = {
    "./assets/sprites/straw-house.png",
    "./assets/sprites/big-house.png",
    "./assets/sprites/tree-house.png"
};

Sint32 main(Sint32 argc, char **argv)
{
    SDL_Window     *window                            = NULL;
    FG_Renderer    *renderer                          = NULL;
    SDL_Surface    *surface                           = NULL;
    FG_Environment  env                               = FG_DEF_ENVIRONMENT;
    FG_Camera       cameras[2]                        = {
        FG_DEF_CAMERA,
        FG_DEF_CAMERA
    };
    const bool     *keys                              = NULL;
    Uint8           i                                 = 0;
    FG_Material     materials[SDL_arraysize(ALBEDOS)] = { 0 };
    FG_Quad3        quad3s[SDL_arraysize(materials)]  = { 0 };
    float           delta                             = 0;
    Uint64          curr_tick                         = 0;
    Uint64          last_tick                         = 0;

    (void)argc;
    (void)argv;

    if (!SDL_SetAppMetadata(__FILE__, "0.0.1", "org.example.flygpu")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    window = SDL_CreateWindow(__FILE__, 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    renderer = FG_CreateRenderer(window, true, true);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    surface = SDL_LoadPNG("./assets/texture.png");
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    if (!FG_RendererCreateTexture(renderer, surface, false, &env.texture) ||
        !env.texture
    ) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    SDL_DestroySurface(surface);

    cameras[0].viewport.br.x = 0.5F;
    cameras[0].env           = &env;

    cameras[1].viewport.tl.x = 0.5F;
    cameras[1].env           = &env;

    keys = SDL_GetKeyboardState(NULL);

    for (i = 0; i != SDL_arraysize(quad3s); ++i) {
        quad3s[i]                 = FG_DEF_QUAD3;
        quad3s[i].transf.transl.z = (float)(i + 1) * -5.0F;

        surface = SDL_LoadPNG(ALBEDOS[i]);
        if (!surface) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        quad3s[i].transf.scale = (FG_Vec2){
            .x = (float)surface->w * 0.01F,
            .y = (float)surface->h * 0.01F
        };

        if (!FG_RendererCreateTexture(
            renderer, surface, false, &materials[i].maps.albedo) ||
            !materials[i].maps.albedo
        ) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        SDL_DestroySurface(surface);

        quad3s[i].material = materials + i;
    }

    while (!SDL_HasEvent(SDL_EVENT_QUIT)) {
        cameras[0].transf.transl.x += (float)(keys[SDL_SCANCODE_D]
                                    - keys[SDL_SCANCODE_A]) * delta * MOVE_SPEED;
        cameras[0].transf.transl.y += (float)(keys[SDL_SCANCODE_SPACE]
                                    - keys[SDL_SCANCODE_LSHIFT]) * delta * MOVE_SPEED;
        cameras[0].transf.transl.z += (float)(keys[SDL_SCANCODE_S]
                                    - keys[SDL_SCANCODE_W]) * delta * MOVE_SPEED;
        cameras[0].transf.rotation += (float)(keys[SDL_SCANCODE_Q]
                                    - keys[SDL_SCANCODE_E]) * delta * ROLL_SPEED;

        cameras[1].transf.transl.x += (float)(keys[SDL_SCANCODE_L]
                                    - keys[SDL_SCANCODE_J]) * delta * MOVE_SPEED;
        cameras[1].transf.transl.y += (float)(keys[SDL_SCANCODE_UP]
                                    - keys[SDL_SCANCODE_DOWN]) * delta * MOVE_SPEED;
        cameras[1].transf.transl.z += (float)(keys[SDL_SCANCODE_K]
                                    - keys[SDL_SCANCODE_I]) * delta * MOVE_SPEED;
        cameras[1].transf.rotation += (float)(keys[SDL_SCANCODE_U]
                                    - keys[SDL_SCANCODE_O]) * delta * ROLL_SPEED;

        if (!FG_RendererDraw(
            renderer,
            &(FG_RendererDrawInfo){
                .camera_count = SDL_arraysize(cameras),
                .cameras      = cameras,
                .quad3_info   = { .count = SDL_arraysize(quad3s), .quad3s = quad3s }
            })
        ) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        SDL_PumpEvents();

        curr_tick = SDL_GetTicks();
        delta     = (float)(curr_tick - last_tick);
        last_tick = curr_tick;
    }

    for (i = 0; i != SDL_arraysize(materials); ++i) {
        FG_RendererDestroyTexture(renderer, materials[i].maps.albedo);
    }
    FG_RendererDestroyTexture(renderer, env.texture);
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
