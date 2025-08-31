/* clang-format off */

/*
  FlyGPU
  Copyright (C) 2025 Domán Zana

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

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include <stdbool.h>
#include <stddef.h>

#define PI 3.1415927410125732421875F

#define DegsToRads(d) ((d) / 180.0F * PI)

static const char *const SPRITES[] = {
    "./assets/sprites/big-house.png",
    "./assets/sprites/house.png",
    "./assets/sprites/palm.png",
    "./assets/sprites/pine.png",
    "./assets/sprites/plant-house.png",
    "./assets/sprites/rock-1.png",
    "./assets/sprites/rock-2.png",
    "./assets/sprites/straw-house.png",
    "./assets/sprites/tree-house.png",
    "./assets/sprites/tree.png",
    "./assets/sprites/wooden-house.png"
};

#define COUNT 8265

Sint32 main(Sint32 argc, char *argv[])
{
    SDL_Window          *window                            = NULL;
    FG_Renderer         *renderer                          = NULL;
    const bool          *keys                              = NULL;
    Uint32               i                                 = 0;
    SDL_Surface         *surface                           = NULL;
    FG_Material          materials[SDL_arraysize(SPRITES)] = { 0 };
    Uint8                j                                 = 0;
    Uint8                k                                 = 0;
    FG_Quad3            *quad3s                            = NULL;
    FG_Quad3            *it                                = NULL;
    FG_Camera            camera                            = {
        .viewport.br     = { .x = 1.0F, .y = 1.0F },
        .perspective     = {
            .fov  = DegsToRads(60.0F),
            .near = 0.1F,
            .far  = 100.0F
        },
        .transform.scale = { .x = 1.0F, .y = 1.0F },
        .mask            = 1
    };
    FG_AmbientLight      ambient                           = {
        .direction.z = -1.0F,
        .color       = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
        .mask        = 1
    };
    FG_RendererDrawInfo  info                              = {
        .cameras          = &camera,
        .camera_count     = 1,
        .quad3_info.count = COUNT * SDL_arraysize(SPRITES) * SDL_arraysize(SPRITES),
        .shading_info = {
            .ambients      = &ambient,
            .ambient_count = 1
        }
    };
    SDL_Event            event                             = { 0 };
    bool                 running                           = true;
    Uint64               tick                              = 0;
    Uint64               delta                             = 0;
    Uint64               last                              = 0;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("FlyGPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    renderer = FG_CreateRenderer(window, 1 < argc && !SDL_strcmp(argv[1], "--vsync"));
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    keys = SDL_GetKeyboardState(NULL);

    for (i = 0; i != SDL_arraysize(SPRITES); ++i) {
        surface = IMG_Load(SPRITES[i]);
        if (!surface) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            return 1;
        }

        FG_RendererCreateTexture(renderer, surface, &materials[i].albedo);
        if (!materials[i].albedo) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            return 1;
        }

        SDL_DestroySurface(surface);
    }

    quad3s = SDL_calloc(info.quad3_info.count, sizeof(FG_Quad3));
    if (!quad3s) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    for (i = 0, it = quad3s; i != COUNT; ++i) {
        for (j = 0; j != SDL_arraysize(SPRITES); ++j) {
            for (k = 0; k != SDL_arraysize(SPRITES); ++k, ++it) {
                it->transform.translation.x = -0.5F + 0.5F / SDL_arraysize(SPRITES)
                                            + (float)k * 1.0F / SDL_arraysize(SPRITES);
                it->transform.translation.y = -0.5F + 0.5F / SDL_arraysize(SPRITES)
                                            + (float)j * 1.0F / SDL_arraysize(SPRITES);
                it->transform.translation.z = -1.0F - (float)i * 0.0001F;
                it->transform.scale.x       = 1.0F / (SDL_arraysize(SPRITES) + 1);
                it->transform.scale.y       = it->transform.scale.x;
                it->color                   = (FG_QuadColor){
                    .tl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
                    .bl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
                    .br = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
                    .tr = { .x = 1.0F, .y = 1.0F, .z = 1.0F }
                };
                it->material                = materials + j;
                it->coords.br               = (FG_Vec2){ .x = 1.0F, .y = 1.0F };
                it->mask                    = 1;
            }
        }
    }

    info.quad3_info.quad3s = quad3s;

    do {
        camera.transform.translation.x += 0.001F * (float)(keys[SDL_SCANCODE_D]
                                        - keys[SDL_SCANCODE_A]) * (float)delta;
        camera.transform.translation.y += 0.001F * (float)(keys[SDL_SCANCODE_SPACE]
                                        - keys[SDL_SCANCODE_LSHIFT]) * (float)delta;
        camera.transform.translation.z += 0.001F * (float)(keys[SDL_SCANCODE_S]
                                        - keys[SDL_SCANCODE_W]) * (float)delta;

        if (!FG_RendererDraw(renderer, &info)) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            return 1;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
        }

        tick  = SDL_GetTicks();
        delta = tick - last;
        last  = tick;

        SDL_Log("DELTA: %lu\n", delta);
    } while (running);

    SDL_free(quad3s);
    for (i = 0; i != SDL_arraysize(SPRITES); ++i) {
        FG_RendererDestroyTexture(renderer, materials[i].albedo);
    }
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
