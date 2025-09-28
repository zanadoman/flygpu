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
#include <SDL3/SDL_main.h>   /* IWYU pragma: keep */
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

Sint32 main(Sint32 argc, char **argv)
{
    SDL_Window  *window   = NULL;
    FG_Renderer *renderer = NULL;

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

    while (!SDL_HasEvent(SDL_EVENT_QUIT)) {
        if (!FG_RendererDraw(renderer, &(FG_RendererDrawInfo){ 0 })) {
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
