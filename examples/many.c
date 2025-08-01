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

/* NOLINTBEGIN(clang-analyzer-unix.Malloc) */

#include "../include/flygpu/flygpu.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#define QUAD3_COUNT 0x100000

#define RandInt(min, max) ((min) + SDL_rand((max) - (min)))
#define RandFloat(min, max) ((min) + SDL_randf() * ((max) - (min)))

static const char *IMAGES[] = {
    "./assets/door-opened.png",
    "./assets/wooden-house.png",
    "./assets/face-block.png",
    "./assets/crank-down.png",
    "./assets/rock-1.png",
    "./assets/crank-up.png",
    "./assets/skulls.png",
    "./assets/small-platform.png",
    "./assets/rock.png",
    "./assets/spikes.png",
    "./assets/pine.png",
    "./assets/rock-2.png",
    "./assets/spikes-top.png",
    "./assets/tree-house.png",
    "./assets/palm.png",
    "./assets/big-house.png",
    "./assets/big-crate.png",
    "./assets/block-big.png",
    "./assets/platform-long.png",
    "./assets/block.png",
    "./assets/spike-skull.png",
    "./assets/shrooms.png",
    "./assets/straw-house.png",
    "./assets/plant-house.png",
    "./assets/sign.png",
    "./assets/house.png",
    "./assets/crate.png",
    "./assets/door.png",
    "./assets/bush.png",
    "./assets/tree.png"
};

Sint32 main(void)
{
    SDL_Window          *window                          = NULL;
    FG_Renderer         *renderer                        = NULL;
    Uint32               i                               = 0;
    SDL_Surface         *surface                         = NULL;
    SDL_GPUTexture      *textures[SDL_arraysize(IMAGES)] = { [0] = NULL };
    FG_Quad3            *quad3s                          = SDL_calloc(QUAD3_COUNT, sizeof(FG_Quad3));
    const bool          *keys                            = NULL;
    Uint64               tick                            = 0;
    bool                 running                         = true;
    SDL_Event            event                           = { .type = SDL_EVENT_FIRST };
    FG_RendererDrawInfo  info                            = {
        .camera      = {
            .perspective = {
                .fov  = FG_DegsToRads(60.0F),
                .near = 0.1F,
                .far  = 100.0F
            },
            .transform.scale = { 1.0F, 1.0F }
        },
        .quad3s_info = {
            .instances = quad3s,
            .count     = QUAD3_COUNT
        }
    };
    Uint64               delta                           = 0;

    if (!quad3s) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("FlyGPU | few", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    renderer = FG_CreateRenderer(window, false);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i != SDL_arraysize(IMAGES); ++i) {
        surface = IMG_Load(IMAGES[i]);
        if (!surface) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return 1;
        }
        if (!FG_RendererCreateTexture(renderer, surface, textures + i)) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
            return 1;
        }
        if (!textures[i]) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return 1;
        }
        SDL_DestroySurface(surface);
    }

    for (i = 0; i != QUAD3_COUNT; ++i) {
       quad3s[i] = (FG_Quad3){
            .transform = {
                .translation = {
                    .x = RandFloat(-10.0F, 10.0F),
                    .y = RandFloat(-10.0F, 10.0F),
                    .z = RandFloat(-10.0F, -30.0F)
                },
                .rotation    = RandFloat(-FG_PI, FG_PI),
                .scale       = {
                    .x = RandFloat(0.5F, 2.5F),
                    .y = RandFloat(0.5F, 2.5F)
                }
            },
            .color     = {
                .bl = {
                    .x = RandFloat(0.0F, 1.0F),
                    .y = RandFloat(0.0F, 1.0F),
                    .z = RandFloat(0.0F, 1.0F),
                    .w = RandFloat(0.0F, 1.0F)
                },
                .br = {
                    .x = RandFloat(0.0F, 1.0F),
                    .y = RandFloat(0.0F, 1.0F),
                    .z = RandFloat(0.0F, 1.0F),
                    .w = RandFloat(0.0F, 1.0F)
                },
                .tr = {
                    .x = RandFloat(0.0F, 1.0F),
                    .y = RandFloat(0.0F, 1.0F),
                    .z = RandFloat(0.0F, 1.0F),
                    .w = RandFloat(0.0F, 1.0F)
                },
                .tl = {
                    .x = RandFloat(0.0F, 1.0F),
                    .y = RandFloat(0.0F, 1.0F),
                    .z = RandFloat(0.0F, 1.0F),
                    .w = RandFloat(0.0F, 1.0F)
                }
            },
            .texture   = textures[RandInt(0, SDL_arraysize(IMAGES))]
        };
    }

    keys = SDL_GetKeyboardState(NULL);

    while (running) {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&event)) if ((event.type) == SDL_EVENT_QUIT) running = false;

        info.camera.transform.translation.x += 0.001F * (float)(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * (float)delta;
        info.camera.transform.translation.y += 0.001F * (float)(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LSHIFT]) * (float)delta;
        info.camera.transform.translation.z += 0.001F * (float)(keys[SDL_SCANCODE_S] - keys[SDL_SCANCODE_W]) * (float)delta;
        info.camera.transform.rotation      += 0.003F * (float)(keys[SDL_SCANCODE_Q] - keys[SDL_SCANCODE_E]) * (float)delta;

        if (!FG_RendererDraw(renderer, &info)) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
            return 1;
        }

        delta = SDL_GetTicks() - tick;
        SDL_Log("Frame time: %lu ms\n", delta);
    }

    SDL_free(quad3s);
    for (i = 0; i != SDL_arraysize(IMAGES); ++i) {
        FG_RendererDestroyTexture(renderer, textures[i]);
    }
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/* NOLINTEND(clang-analyzer-unix.Malloc) */
