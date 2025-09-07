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
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include <stdbool.h>
#include <stddef.h>

#define PI 3.1415927410125732421875F

#define DegsToRads(d) ((d) / 180.0F * PI)

static const char *const MATERIAL_INFOS[][3] = {
    {
        "./assets/albedos/leather.png",
        "./assets/speculars/leather.png",
        "./assets/normals/leather.png"
    },
    {
        "./assets/albedos/metal.png",
        "./assets/speculars/metal.png",
        "./assets/normals/metal.png"
    },
    {
        "./assets/albedos/rock.png",
        "./assets/speculars/rock.png",
        "./assets/normals/rock.png"
    }
};

Sint32 main(Sint32 argc, char *argv[])
{
    SDL_Window          *window                                   = NULL;
    FG_Renderer         *renderer                                 = NULL;
    SDL_Surface         *surface                                  = NULL;
    FG_Material          materials[SDL_arraysize(MATERIAL_INFOS)] = { 0 };
    Sint32               i                                        = 0;
    Sint32               width                                    = 0;
    Sint32               height                                   = 0;
    float                x                                        = 0.0F;
    float                y                                        = 0.0F;
    FG_Camera            camera                                   = {
        .viewport.br     = { .x = 1.0F, .y = 1.0F },
        .perspective     = {
            .fov  = DegsToRads(60.0F),
            .near = 0.1F,
            .far  = 100.0F
        },
        .transform.scale = { .x = 1.0F, .y = 1.0F },
        .mask            = 1
    };
    FG_Quad3             quad3                                    = {
        .transform = {
            .translation.z = -1.0F,
            .scale           = { .x = 1.0F, .y = 1.0F }
        },
        .color     = {
            .tl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .bl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .br = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .tr = { .x = 1.0F, .y = 1.0F, .z = 1.0F }
        },
        .material  = materials,
        .coords.br = { .x = 1.0F, .y = 1.0F },
        .mask      = 1
    };
    FG_DirectLight      ambient                                  = {
        .direction.z = -1.0F,
        .color       = { .x = 0.5F, .y = 0.5F, .z = 0.5F },
        .mask        = 1
    };
    FG_OmniLight         omni                                     = {
        .translation.z = -0.75F,
        .color         = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
        .radius        = 0.5F,
        .mask          = 1
    };
    FG_RendererDrawInfo  info                                     = {
        .cameras      = &camera,
        .camera_count = 1,
        .quad3_info   = {
            .quad3s = &quad3,
            .count     = 1
        },
        .shading_info = {
            .directs      = &ambient,
            .direct_count = 1,
            .omnis         = &omni,
            .omni_count    = 1,
        }
    };
    SDL_Event            event                                    = { 0 };
    bool                 running                                  = true;
    Uint64               tick                                     = 0;
    Uint64               delta                                    = 0;
    Uint64               last                                     = 0;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("FlyGPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    renderer = FG_CreateRenderer(window, 1 < argc && !SDL_strcmp(argv[1], "--vsync"), true);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i != SDL_arraysize(materials); ++i) {
        if (MATERIAL_INFOS[i][0]) {
            surface = IMG_Load(MATERIAL_INFOS[i][0]);
            if (!renderer) {
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

        if (MATERIAL_INFOS[i][1]) {
            surface = IMG_Load(MATERIAL_INFOS[i][1]);
            if (!renderer) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                return 1;
            }

            FG_RendererCreateTexture(renderer, surface, &materials[i].specular);
            if (!materials[i].albedo) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                return 1;
            }

            SDL_DestroySurface(surface);
        }

        if (MATERIAL_INFOS[i][2]) {
            surface = IMG_Load(MATERIAL_INFOS[i][2]);
            if (!renderer) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                return 1;
            }

            FG_RendererCreateTexture(renderer, surface, &materials[i].normal);
            if (!materials[i].albedo) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                return 1;
            }

            SDL_DestroySurface(surface);
        }
    }

    i = 0;

    SDL_SetWindowRelativeMouseMode(window, true);

    do {
        if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            return 1;
        }

        SDL_GetRelativeMouseState(&x, &y);

        omni.translation.x += x / (float)width;
        omni.translation.x  = SDL_clamp(omni.translation.x, -0.5F, 0.5F);

        omni.translation.y += -y / (float)height;
        omni.translation.y  = SDL_clamp(omni.translation.y, -0.5F, 0.5F);

        if (!FG_RendererDraw(renderer, &info)) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            return 1;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                i = i + (Sint32)event.wheel.y;
                if (i < 0) i = SDL_arraysize(materials) - 1;
                else if (SDL_arraysize(materials) <= (size_t)i) i = 0;
                quad3.material = materials + i;
            }
        }

        tick  = SDL_GetTicks();
        delta = tick - last;
        last  = tick;

        SDL_Log("DELTA: %lu\n", delta);
    } while (running);

    for (i = 0; i != SDL_arraysize(materials); ++i) {
        FG_RendererDestroyTexture(renderer, materials[i].normal);
        FG_RendererDestroyTexture(renderer, materials[i].specular);
        FG_RendererDestroyTexture(renderer, materials[i].albedo);
    }
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
