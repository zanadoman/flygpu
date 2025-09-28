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
#include <SDL3/SDL_main.h>   /* IWYU pragma: keep */
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define SIZE     0.49F
#define POSITION ((1.0F + SIZE) / 6.0F)

Sint32 main(Sint32 argc, char **argv)
{
    SDL_Window  *window    = NULL;
    FG_Renderer *renderer  = NULL;
    FG_Quad3     quad3s[4] = {
        FG_DEF_QUAD3,
        FG_DEF_QUAD3,
        FG_DEF_QUAD3,
        FG_DEF_QUAD3
    };
    FG_Camera    camera    = FG_DEF_CAMERA;

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

    renderer = FG_CreateRenderer(window, false, true);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    quad3s[0].transform.translation = (FG_Vec3){
        .x = -POSITION,
        .y = POSITION,
        .z = -1.0F
    };
    quad3s[0].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[0].color.bl.x            = 0.0F;
    quad3s[0].color.bl.y            = 0.0F;
    quad3s[0].color.br.x            = 0.0F;
    quad3s[0].color.br.z            = 0.0F;
    quad3s[0].color.tr.y            = 0.0F;
    quad3s[0].color.tr.z            = 0.0F;

    quad3s[1].transform.translation = (FG_Vec3){
        .x = -POSITION,
        .y = -POSITION,
        .z = -1.0F
    };
    quad3s[1].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[1].color.tl.x            = 0.0F;
    quad3s[1].color.tl.y            = 0.0F;
    quad3s[1].color.bl.x            = 0.0F;
    quad3s[1].color.bl.z            = 0.0F;
    quad3s[1].color.br.x            = 0.0F;
    quad3s[1].color.br.y            = 0.0F;
    quad3s[1].color.tr.x            = 0.0F;
    quad3s[1].color.tr.z            = 0.0F;

    quad3s[2].transform.translation = (FG_Vec3){
        .x = POSITION,
        .y = -POSITION,
        .z = -1.0F
    };
    quad3s[2].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[2].color.tl.x            = 0.0F;
    quad3s[2].color.tl.z            = 0.0F;
    quad3s[2].color.bl.x            = 0.0F;
    quad3s[2].color.bl.y            = 0.0F;
    quad3s[2].color.br.x            = 0.0F;
    quad3s[2].color.br.z            = 0.0F;
    quad3s[2].color.tr.x            = 0.0F;
    quad3s[2].color.tr.y            = 0.0F;

    quad3s[3].transform.translation = (FG_Vec3){
        .x = POSITION,
        .y = POSITION,
        .z = -1.0F
    };
    quad3s[3].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[3].color.tl.y            = 0.0F;
    quad3s[3].color.tl.z            = 0.0F;
    quad3s[3].color.bl.x            = 0.0F;
    quad3s[3].color.bl.z            = 0.0F;
    quad3s[3].color.br.x            = 0.0F;
    quad3s[3].color.br.y            = 0.0F;

    while (!SDL_HasEvent(SDL_EVENT_QUIT)) {
        if (!FG_RendererDraw(renderer, &(FG_RendererDrawInfo){
            .camera_count = 1,
            .cameras      = &camera,
            .quad3_info   = { .count = SDL_arraysize(quad3s), .quad3s = quad3s }
        })) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
            abort();
        }

        SDL_PumpEvents();
    }

    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
