#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include "../include/flygpu/flygpu.h"

Sint32 main(void)
{
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("SDL_GPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return 1;
    }

    FG_Renderer *renderer = FG_CreateRenderer(window, false);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s\n", SDL_GetError());
        return 1;
    }

    FG_Quad3 quads[2] = {
        [0] = {.transform = {.t.z = -1.0F, .s.x = 0.1F, .s.y = 0.1F}},
        [1] = {.transform = {.t.x = 0.5F,
                             .t.y = 0.5F,
                             .t.z = -5.0F,
                             .rotation   = -45.0F / 180.0F * 3.14F,
                             .s.x = 1.2F,
                             .s.y = 1.0F}}};

    bool   running = true;
    Uint64 tick    = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if ((event.type) == SDL_EVENT_QUIT) running = false;
        }

        if (!FG_RendererDraw(renderer, quads,
                             quads + sizeof(quads) / sizeof(*quads))) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
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
