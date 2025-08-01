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

Sint32 main(void)
{
    SDL_Window          *window    = NULL;
    FG_Renderer         *renderer  = NULL;
    SDL_Surface         *surface   = NULL;
    FG_Quad3             quad3s[3] = {
        [0] = {
            .transform = {
                .translation = { 0.0F, 0.0F, -1.0F },
                .scale       = { 0.5525F, 0.468F }
            },
            .color     = {
                .bl = { 1.0F, 1.0F, 1.0F, 1.0F },
                .br = { 1.0F, 1.0F, 1.0F, 1.0F },
                .tr = { 1.0F, 1.0F, 1.0F, 1.0F },
                .tl = { 1.0F, 1.0F, 1.0F, 1.0F }
            }
        },
        [1] = {
            .transform = {
                .translation = { 2.0F, 2.0F, -10.0F },
                .rotation    = FG_DegsToRads(-45.0F),
                .scale       = { 3.0F, 1.0F }
            },
            .color     = {
                .bl = { 1.0F, 0.0F, 0.0F, 1.0F },
                .br = { 0.0F, 1.0F, 0.0F, 1.0F },
                .tr = { 0.0F, 0.0F, 1.0F, 1.0F },
                .tl = { 1.0F, 1.0F, 1.0F, 1.0F }
            }
        },
        [2] = {
            .transform = {
                .translation = { -2.0F, 0.0F, -5.0F },
                .scale       = { 1.23F, 1.95F }
            },
            .color     = {
                .bl = { 1.0F, 0.0F, 0.0F, 1.0F },
                .br = { 1.0F, 0.0F, 0.0F, 1.0F },
                .tr = { 0.0F, 0.0F, 1.0F, 1.0F },
                .tl = { 0.0F, 0.0F, 1.0F, 1.0F }
            }
        }
    };
    const bool          *keys      = NULL;
    Uint64               tick      = 0;
    bool                 running   = true;
    SDL_Event            event     = { .type = SDL_EVENT_FIRST };
    FG_RendererDrawInfo  info      = {
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
            .count     = SDL_arraysize(quad3s)
        }
    };
    Uint64               delta     = 0;

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

    surface = IMG_Load("./assets/big-house.png");
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    if (!FG_RendererCreateTexture(renderer, surface, &quad3s[0].texture)) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }
    if (!quad3s[0].texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_DestroySurface(surface);

    surface = IMG_Load("./assets/block-big.png");
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    if (!FG_RendererCreateTexture(renderer, surface, &quad3s[1].texture)) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }
    if (!quad3s[1].texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_DestroySurface(surface);

    surface = IMG_Load("./assets/pine.png");
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    if (!FG_RendererCreateTexture(renderer, surface, &quad3s[2].texture)) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }
    if (!quad3s[2].texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_DestroySurface(surface);

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

    FG_RendererDestroyTexture(renderer, quad3s[2].texture);
    FG_RendererDestroyTexture(renderer, quad3s[1].texture);
    FG_RendererDestroyTexture(renderer, quad3s[0].texture);
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
