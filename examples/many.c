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
    "./assets/albedos/door-opened.png",
    "./assets/albedos/wooden-house.png",
    "./assets/albedos/face-block.png",
    "./assets/albedos/crank-down.png",
    "./assets/albedos/rock-1.png",
    "./assets/albedos/crank-up.png",
    "./assets/albedos/skulls.png",
    "./assets/albedos/small-platform.png",
    "./assets/albedos/rock.png",
    "./assets/albedos/spikes.png",
    "./assets/albedos/pine.png",
    "./assets/albedos/rock-2.png",
    "./assets/albedos/spikes-top.png",
    "./assets/albedos/tree-house.png",
    "./assets/albedos/palm.png",
    "./assets/albedos/big-house.png",
    "./assets/albedos/big-crate.png",
    "./assets/albedos/block-big.png",
    "./assets/albedos/platform-long.png",
    "./assets/albedos/block.png",
    "./assets/albedos/spike-skull.png",
    "./assets/albedos/shrooms.png",
    "./assets/albedos/straw-house.png",
    "./assets/albedos/plant-house.png",
    "./assets/albedos/sign.png",
    "./assets/albedos/house.png",
    "./assets/albedos/crate.png",
    "./assets/albedos/door.png",
    "./assets/albedos/bush.png",
    "./assets/albedos/tree.png"
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
    FG_Camera            camera                          = {
        .viewport        = { { 0.0F, 0.0F },  { 1.0F, 1.0F } },
        .perspective     = {
            .fov  = FG_DegsToRads(60.0F),
            .near = 0.1F,
            .far  = 100.0F
        },
        .transform.scale = { 1.0F, 1.0F }
    };
    FG_RendererDrawInfo  info                            = {
        .cameras       = &camera,
        .camera_count  = 1,
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
                .tl = {
                    .x = RandFloat(0.0F, 1.0F),
                    .y = RandFloat(0.0F, 1.0F),
                    .z = RandFloat(0.0F, 1.0F),
                    .w = RandFloat(0.0F, 1.0F)
                },
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
                }
            },
            .albedo    = textures[RandInt(0, SDL_arraysize(IMAGES))],
            .coords    = {
                .tl = {
                    .x = RandFloat(0.0F, 2.0F),
                    .y = RandFloat(0.0F, 2.0F)
                },
                .br = {
                    .x = RandFloat(0.0F, 2.0F),
                    .y = RandFloat(0.0F, 2.0F)
                }
            }
        };
    }

    keys = SDL_GetKeyboardState(NULL);

    while (running) {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&event)) if ((event.type) == SDL_EVENT_QUIT) running = false;

        camera.transform.translation.x += 0.001F * (float)(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * (float)delta;
        camera.transform.translation.y += 0.001F * (float)(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LSHIFT]) * (float)delta;
        camera.transform.translation.z += 0.001F * (float)(keys[SDL_SCANCODE_S] - keys[SDL_SCANCODE_W]) * (float)delta;
        camera.transform.rotation      += 0.003F * (float)(keys[SDL_SCANCODE_Q] - keys[SDL_SCANCODE_E]) * (float)delta;

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
    if (!FG_DestroyRenderer(renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
