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

#ifndef EXAMPLES_COMMON_H
#define EXAMPLES_COMMON_H

#ifndef WIDTH
#define WIDTH 800
#endif /* WIDTH */

#ifndef HEIGHT
#define HEIGHT 600
#endif /* HEIGHT */

#ifndef VSYNC
#define VSYNC false
#endif /* VSYNC */

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#ifdef USE_MATERIALS
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#endif /* USE_MATERIALS */

#include <stdbool.h>
#ifdef USE_MATERIALS
#include <stddef.h>
#endif /* USE_MATERIALS */

static FG_RendererDrawInfo  INFO;
static SDL_Window          *WINDOW;
static FG_Renderer         *RENDERER;
static FG_Camera           *CAMERAS;
static FG_Quad3            *QUAD3S;
static FG_AmbientLight     *AMBIENTS;
static FG_OmniLight        *OMNIS;
#ifdef USE_MATERIALS
static FG_Material          MATERIALS[SDL_arraysize(MATERIAL_INFOS)];
#endif /* USE_MATERIALS */
static const bool          *KEYS;
static float                DELTA;

#define Check(x)                                                      \
do {                                                                  \
    if (!(x)) {                                                       \
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError()); \
        return 1;                                                     \
    }                                                                 \
} while (0)

#define DefaultCamera(camera)              \
do {                                       \
    (camera) = (FG_Camera){                \
        .viewport.br     = { 1.0F, 1.0F }, \
        .perspective     = {               \
            .fov  = FG_DegsToRads(60.0F),  \
            .near = 0.1F,                  \
            .far  = 100.0F                 \
        },                                 \
        .transform.scale = { 1.0F, 1.0F }, \
        .mask            = 0xFFFFFFFF      \
    };                                     \
} while (0)

#define DefaultQuad3(quad3)                \
do {                                       \
    (quad3) = (FG_Quad3){                  \
        .transform.scale = { 1.0F, 1.0F }, \
        .color           = {               \
            { 1.0F, 1.0F, 1.0F },          \
            { 1.0F, 1.0F, 1.0F },          \
            { 1.0F, 1.0F, 1.0F },          \
            { 1.0F, 1.0F, 1.0F }           \
        },                                 \
        .coords.br       = { 1.0F, 1.0F }  \
    };                                     \
} while (0)

#define DefaultAmbient(ambient)             \
do {                                        \
    (ambient) = (FG_AmbientLight){          \
        .direction = { 0.0F, 0.0F, -1.0F }, \
        .color     = { 1.0F, 1.0F, 1.0F },  \
        .mask      = 0xFFFFFFFF             \
    };                                      \
} while (0)

#define DefaultOmni(omni)               \
do {                                    \
    (omni) = (FG_OmniLight){            \
        .radius = 1.0F,                 \
        .color  = { 1.0F, 1.0F, 1.0F }, \
        .mask   = 0xFFFFFFFF            \
    };                                  \
} while (0)

#define Init()                                                                     \
do {                                                                               \
    Uint32 i = 0;                                                                  \
    if (INFO.camera_count) {                                                       \
        CAMERAS = SDL_calloc(INFO.camera_count, sizeof(*CAMERAS));                 \
        Check(CAMERAS);                                                            \
        for (i = 0; i != INFO.camera_count; ++i) DefaultCamera(CAMERAS[i]);        \
    }                                                                              \
    if (INFO.quad3_info.count) {                                                   \
        QUAD3S = SDL_calloc(INFO.quad3_info.count, sizeof(*QUAD3S));               \
        Check(QUAD3S);                                                             \
        for (i = 0; i != INFO.quad3_info.count; ++i) DefaultQuad3(QUAD3S[i]);      \
    }                                                                              \
    if (INFO.shading_info.ambient_count) {                                         \
        AMBIENTS = SDL_calloc(INFO.shading_info.ambient_count, sizeof(*AMBIENTS)); \
        Check(AMBIENTS);                                                           \
        for (i = 0; i != INFO.shading_info.ambient_count; ++i) {                   \
            DefaultAmbient(AMBIENTS[i]);                                           \
        }                                                                          \
    }                                                                              \
    if (INFO.shading_info.omni_count) {                                            \
        OMNIS = SDL_calloc(INFO.shading_info.omni_count, sizeof(*OMNIS));          \
        Check(OMNIS);                                                              \
        for (i = 0; i != INFO.shading_info.omni_count; ++i) DefaultOmni(OMNIS[i]); \
    }                                                                              \
    Check(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS));                    \
    WINDOW = SDL_CreateWindow("FlyGPU", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);      \
    Check(WINDOW);                                                                 \
    RENDERER = FG_CreateRenderer(WINDOW, VSYNC);                                   \
    Check(RENDERER);                                                               \
    KEYS = SDL_GetKeyboardState(NULL);                                             \
} while (0)

#ifdef USE_MATERIALS
#define InitMaterials()                                                          \
do {                                                                             \
    Uint32       i       = 0;                                                    \
    SDL_Surface *surface = NULL;                                                 \
    for (i = 0; i != SDL_arraysize(MATERIAL_INFOS); ++i) {                       \
        if (MATERIAL_INFOS[i][0]) {                                              \
            surface = IMG_Load(MATERIAL_INFOS[i][0]);                            \
            Check(surface);                                                      \
            FG_RendererCreateTexture(RENDERER, surface, &MATERIALS[i].albedo);   \
            Check(MATERIALS[i].albedo);                                          \
            SDL_DestroySurface(surface);                                         \
        }                                                                        \
        if (MATERIAL_INFOS[i][1]) {                                              \
            surface = IMG_Load(MATERIAL_INFOS[i][1]);                            \
            Check(surface);                                                      \
            FG_RendererCreateTexture(RENDERER, surface, &MATERIALS[i].specular); \
            Check(MATERIALS[i].specular);                                        \
            SDL_DestroySurface(surface);                                         \
        }                                                                        \
        if (MATERIAL_INFOS[i][2]) {                                              \
            surface = IMG_Load(MATERIAL_INFOS[i][2]);                            \
            Check(surface);                                                      \
            FG_RendererCreateTexture(RENDERER, surface, &MATERIALS[i].normal);   \
            Check(MATERIALS[i].normal);                                          \
            SDL_DestroySurface(surface);                                         \
        }                                                                        \
    }                                                                            \
} while (0)
#endif /* USE_MATERIALS */

#define loop while (PollEvents())

#define Update()                             \
do {                                         \
    static Uint64 last;                      \
    Uint64        now;                       \
    INFO.cameras               = CAMERAS;    \
    INFO.quad3_info.instances  = QUAD3S;     \
    INFO.shading_info.ambients = AMBIENTS;   \
    INFO.shading_info.omnis    = OMNIS;      \
    Check(FG_RendererDraw(RENDERER, &INFO)); \
    now = SDL_GetTicks();                    \
    DELTA = (float)(now - last);             \
    last = now;                              \
    SDL_Log("DELTA: %.0f\n", (double)DELTA); \
} while (0)

#ifdef USE_MATERIALS
#define DestroyMaterials()                                          \
do {                                                                \
    Uint32 i = 0;                                                   \
    for (i = 0; i != SDL_arraysize(MATERIAL_INFOS); ++i) {          \
        FG_RendererDestroyTexture(RENDERER, MATERIALS[i].normal);   \
        FG_RendererDestroyTexture(RENDERER, MATERIALS[i].specular); \
        FG_RendererDestroyTexture(RENDERER, MATERIALS[i].albedo);   \
    }                                                               \
} while (0)
#endif /* USE_MATERIALS */

#define Quit()                                   \
do {                                             \
    SDL_free(OMNIS);                             \
    SDL_free(AMBIENTS);                          \
    SDL_free(QUAD3S);                            \
    SDL_free(CAMERAS);                           \
    if (!FG_DestroyRenderer(RENDERER)) return 1; \
    SDL_DestroyWindow(WINDOW);                   \
    SDL_Quit();                                  \
    return 0;                                    \
} while (0)

#define Action(x, y) (float)(KEYS[SDL_SCANCODE_##x] - KEYS[SDL_SCANCODE_##y])

#define MoveCamera(i, ms, rs, mr, ml, mu, md, mb, mf, rl, rr)            \
do {                                                                     \
    CAMERAS[i].transform.translation.x += (ms) * Action(mr, ml) * DELTA; \
    CAMERAS[i].transform.translation.y += (ms) * Action(mu, md) * DELTA; \
    CAMERAS[i].transform.translation.z += (ms) * Action(mb, mf) * DELTA; \
    CAMERAS[i].transform.rotation      += (rs) * Action(rl, rr) * DELTA; \
} while (0)

bool PollEvents(void);

bool PollEvents(void)
{
    SDL_Event event = { .type = SDL_EVENT_FIRST };

    while (SDL_PollEvent(&event)) if (event.type == SDL_EVENT_QUIT) return false;

    return true;
}

#endif /* EXAMPLES_COMMON_H */
