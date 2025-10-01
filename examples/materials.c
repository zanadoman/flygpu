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
#include "../include/flygpu/macros.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>        /* IWYU pragma: keep */
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

static const char *const MATERIALS[3][SDL_arraysize(((FG_Material *)0)->iter)] = {
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

Sint32 main(Sint32 argc, char **argv)
{
    SDL_Window     *window                              = NULL;
    FG_Renderer    *renderer                            = NULL;
    FG_Environment  env                                 = FG_DEF_ENVIRONMENT;
    FG_Camera       camera                              = FG_DEF_CAMERA;
    Uint8           i                                   = 0;
    Uint8           j                                   = 0;
    SDL_Surface    *surface                             = NULL;
    FG_Material     materials[SDL_arraysize(MATERIALS)] = { 0 };
    FG_Quad3        quad3                               = FG_DEF_QUAD3;
    FG_DirectLight  direct                              = FG_DEF_DIRECT_LIGHT;
    FG_OmniLight    omni                                = FG_DEF_OMNI_LIGHT;
    SDL_Event       event                               = { 0 };
    Uint32          buttons                             = 0;
    float           x                                   = 0.0F;
    float           y                                   = 0.0F;
    Sint32          width                               = 0;
    Sint32          height                              = 0;

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

    window = SDL_CreateWindow(__FILE__, 720, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    renderer = FG_CreateRenderer(window, true, true);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    env.light = (FG_Vec3){ .x = 0.0F, .y = 0.0F, .z = 0.0F };

    camera.env = &env;

    for (i = 0; i != SDL_arraysize(MATERIALS); ++i) {
        for (j = 0; j != SDL_arraysize(MATERIALS[i]); ++j) {
            surface = IMG_Load(MATERIALS[i][j]);
            if (!surface) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                abort();
            }

            FG_RendererCreateTexture(renderer, surface, i != 0, materials[i].iter + j);
            if (!materials[i].iter[j]) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
                abort();
            }

            SDL_DestroySurface(surface);
        }
    }

    quad3.transf.transl.z = -1.0F;
    quad3.transf.scale    = (FG_Vec2){ .x = 0.9F, .y = 0.9F };
    quad3.material        = materials;

    direct.color = (FG_Vec3){ .x = 0.5F, .y = 0.5F, .z = 0.5F };

    omni.transl.z = -0.75F;
    omni.radius   = 0.5F;
    omni.color    = (FG_Vec3){ .x = 0.75F, .y = 0.75F, .z = 0.75F };

    while (!SDL_HasEvent(SDL_EVENT_QUIT)) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                quad3.material += (Sint32)event.wheel.y;
                if (quad3.material < materials) {
                    quad3.material = SDL_arraysize(materials) - 1 + materials;
                }
                else if (materials + SDL_arraysize(materials) <= quad3.material) {
                    quad3.material = materials;
                }
                break;
            }
        }

        buttons = SDL_GetMouseState(&x, &y);

        x = (x / (float)width - 0.5F) * ((float)width / (float)height);
        y = -y / (float)height + 0.5F;

        if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        if ((buttons & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) {
            omni.transl.x = x;
            omni.transl.y = y;
        }

        if ((buttons & SDL_BUTTON_RMASK) == SDL_BUTTON_RMASK) {
            direct.direction.x = x;
            direct.direction.y = y;
        }

        if (!FG_RendererDraw(
            renderer,
            &(FG_RendererDrawInfo){
                .camera_count = 1,
                .cameras      = &camera,
                .quad3_info   = { .count = 1, .quad3s = &quad3 },
                .shading_info = {
                    .direct_count = 1,
                    .omni_count   = 1,
                    .directs      = &direct,
                    .omnis        = &omni
                }
            })
        ) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        SDL_PumpEvents();
    }

    for (i = 0; i != SDL_arraysize(materials); ++i) {
        for (j = 0; j != SDL_arraysize(materials[i].iter); ++j) {
            FG_RendererDestroyTexture(renderer, materials[i].iter[j]);
        }
    }
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
