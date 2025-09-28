#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/macros.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>   /* IWYU pragma: keep */
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define SIZE     0.4F
#define POSITION ((1.0F + SIZE) / 6.0F)
#define FRAMES   7
#define DURATION 200

Sint32 main(Sint32 argc, char **argv)
{
    SDL_Window  *window    = NULL;
    FG_Renderer *renderer  = NULL;
    SDL_Surface *surface   = NULL;
    FG_Material  material  = { 0 };
    FG_Quad3     quad3s[4] = {
        FG_DEF_QUAD3,
        FG_DEF_QUAD3,
        FG_DEF_QUAD3,
        FG_DEF_QUAD3
    };
    Uint8        i         = 0;
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

    surface = IMG_Load("./assets/animations/explosion.png");
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    FG_RendererCreateTexture(renderer, surface, &material.albedo);
    if (!material.albedo) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
        abort();
    }

    SDL_DestroySurface(surface);

    quad3s[0].transform.translation = (FG_Vec3){
        .x = -POSITION,
        .y = POSITION,
        .z = -1.0F
    };
    quad3s[0].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[0].material              = &material;

    quad3s[1].transform.translation = (FG_Vec3){
        .x = -POSITION,
        .y = -POSITION,
        .z = -1.0F
    };
    quad3s[1].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[1].material              = &material;

    quad3s[2].transform.translation = (FG_Vec3){
        .x = POSITION,
        .y = -POSITION,
        .z = -1.0F
    };
    quad3s[2].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[2].material              = &material;

    quad3s[3].transform.translation = (FG_Vec3){
        .x = POSITION,
        .y = POSITION,
        .z = -1.0F
    };
    quad3s[3].transform.scale       = (FG_Vec2){ .x = SIZE, .y = SIZE };
    quad3s[3].material              = &material;

    while (!SDL_HasEvent(SDL_EVENT_QUIT)) {
        for (i = 0; i != SDL_arraysize(quad3s); ++i) {
            quad3s[i].coords.tl.x = (float)((SDL_GetTicks() + FRAMES * DURATION
                                  / SDL_arraysize(quad3s) * i) / DURATION % FRAMES)
                                  / FRAMES;
            quad3s[i].coords.br.x = quad3s[i].coords.tl.x + 1.0F / FRAMES;
        }

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

    FG_RendererDestroyTexture(renderer, material.albedo);
    FG_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
