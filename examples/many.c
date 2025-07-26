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
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#define QUAD3S_SIZE 10000

#define NextRand(min, max) ((min) + SDL_randf() * ((max) - (min)))

Sint32 main(void)
{
    SDL_Window          *window   = NULL;
    FG_Renderer         *renderer = NULL;
    FG_Quad3            *quad3s   = NULL;
    Sint32               i        = 0;
    Uint64               tick     = 0;
    bool                 running  = true;
    SDL_Event            event;
    FG_RendererDrawInfo  info     = { .quad3s_info.count = QUAD3S_SIZE };

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("SDL_GPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    renderer = FG_CreateRenderer(window, false);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }

    quad3s = SDL_calloc(QUAD3S_SIZE, sizeof(*quad3s));
    info.quad3s_info.insts = quad3s;
    if (!quad3s) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i != QUAD3S_SIZE; ++i) {
        quad3s[i] = (FG_Quad3){
            .transform = {
                .translation = {
                    .x = NextRand(-2.0F, 2.0F),
                    .y = NextRand(-2.0F, 2.0F),
                    .z = NextRand(-1.0F, -3.0F)
                },
                .rotation    = NextRand(-FG_PI, FG_PI),
                .scale       = {
                    .x = NextRand(0.5F, 2.5F),
                    .y = NextRand(0.5F, 2.5F)
                }
            },
            .color     = {
                .bl = {
                    .x = NextRand(0.0F, 1.0F),
                    .y = NextRand(0.0F, 1.0F),
                    .z = NextRand(0.0F, 1.0F),
                    .w = NextRand(0.0F, 1.0F)
                },
                .br = {
                    .x = NextRand(0.0F, 1.0F),
                    .y = NextRand(0.0F, 1.0F),
                    .z = NextRand(0.0F, 1.0F),
                    .w = NextRand(0.0F, 1.0F)
                },
                .tr = {
                    .x = NextRand(0.0F, 1.0F),
                    .y = NextRand(0.0F, 1.0F),
                    .z = NextRand(0.0F, 1.0F),
                    .w = NextRand(0.0F, 1.0F)
                },
                .tl = {
                    .x = NextRand(0.0F, 1.0F),
                    .y = NextRand(0.0F, 1.0F),
                    .z = NextRand(0.0F, 1.0F),
                    .w = NextRand(0.0F, 1.0F)
                }
            }
        };
    }

    tick = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if ((event.type) == SDL_EVENT_QUIT) running = false;
        }

        if (!FG_RendererDraw(renderer, &info)) {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
            return 1;
        }

        SDL_Log("Frame time: %lu ms\n", SDL_GetTicks() - tick);
        tick = SDL_GetTicks();
    }

    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
