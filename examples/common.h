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

#include "../include/flygpu/flygpu.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_scancode.h>
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

#define EX_PI 3.1415927410125732421875F

#define EX_DegsToRads(d) ((d) / 180.0F * EX_PI)

#define EX_RadsToDegs(r) ((r) * 180.0F / EX_PI)

#define EX_DEF_CAMERA (FG_Camera){               \
    .viewport.br     = { .x = 1.0F, .y = 1.0F }, \
    .perspective     = {                         \
        .fov  = EX_DegsToRads(60.0F),            \
        .near = 0.1F,                            \
        .far  = 100.0F                           \
    },                                           \
    .transform.scale = { .x = 1.0F, .y = 1.0F }, \
    .mask            = 0xFFFFFFFF                \
}

#define EX_DEF_QUAD3 (FG_Quad3){                   \
    .transform.scale = { .x = 1.0F, .y = 1.0F },   \
    .color           = {                           \
        .tl = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .bl = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .br = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .tr = { .x = 1.0F, .y = 1.0F, .z = 1.0F }  \
    },                                             \
    .coords.br       = { .x = 1.0F, .y = 1.0F }    \
}

#define EX_DEF_AMBIENT (FG_AmbientLight){              \
    .direction = { .x = 0.0F, .y = 0.0F, .z = -1.0F }, \
    .color     = { .x = 1.0F, .y = 1.0F, .z = 1.0F },  \
    .mask      = 0xFFFFFFFF                            \
}

#define EX_DEF_OMNI (FG_OmniLight){                \
    .radius = 1.0F,                                \
    .color  = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
    .mask   = 0xFFFFFFFF                           \
}

#define EX_Check(r)                                                   \
do {                                                                  \
    if (!(r)) {                                                       \
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError()); \
        return 1;                                                     \
    }                                                                 \
} while (0)

static SDL_Window          *EX_WINDOW;
static FG_Renderer         *EX_RENDERER;
#ifdef USE_MATERIALS
static FG_Material          EX_MATERIALS[SDL_arraysize(MATERIAL_INFOS)];
#endif /* USE_MATERIALS */
static Uint32               EX_CAMERA_COUNT                              = CAMERA_COUNT;
static FG_Camera           *EX_CAMERAS;
static Uint32               EX_QUAD3_COUNT                               = QUAD3_COUNT;
static FG_Quad3            *EX_QUAD3S;
static Uint32               EX_AMBIENT_COUNT                             = AMBIENT_COUNT;
static FG_AmbientLight     *EX_AMBIENTS;
static Uint32               EX_OMNI_COUNT                                = OMNI_COUNT;
static FG_OmniLight        *EX_OMNIS;
static const bool          *EX_KEYS;
static float                EX_DELTA;

Sint32 EX_RandInt(Sint32 min, Sint32 max);

float EX_RandFloat(float min, float max);

bool EX_Init(Sint32 argc, char *argv[]);

bool EX_PollQuit(void);

bool EX_Update(void);

bool EX_Quit(void);

Sint32 EX_RandInt(Sint32 min, Sint32 max)
{
    return min + SDL_rand(max - min);
}

float EX_RandFloat(float min, float max)
{
    return min + SDL_randf() * (max - min);
}

bool EX_Init(Sint32 argc, char *argv[])
{
    Uint32       i       = 0;
#ifdef USE_MATERIALS
    SDL_Surface *surface = NULL;
#endif /* USE_MATERIALS */
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return false;
    EX_WINDOW = SDL_CreateWindow("FlyGPU", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!EX_WINDOW) return false;
    EX_RENDERER = FG_CreateRenderer(
        EX_WINDOW, 1 < argc && !SDL_strcmp(argv[1], "--vsync"));
    if (!EX_RENDERER) return false;
#ifdef USE_MATERIALS
    for (i = 0; i != SDL_arraysize(MATERIAL_INFOS); ++i) {
        if (MATERIAL_INFOS[i][0]) {
            surface = IMG_Load(MATERIAL_INFOS[i][0]);
            if (!surface) return false;
            FG_RendererCreateTexture(EX_RENDERER, surface, &EX_MATERIALS[i].albedo);
            if (!EX_MATERIALS[i].albedo) return false;
            SDL_DestroySurface(surface);
        }
        if (MATERIAL_INFOS[i][1]) {
            surface = IMG_Load(MATERIAL_INFOS[i][1]);
            if (!surface) return false;
            FG_RendererCreateTexture(EX_RENDERER, surface, &EX_MATERIALS[i].specular);
            if (!EX_MATERIALS[i].specular) return false;
            SDL_DestroySurface(surface);
        }
        if (MATERIAL_INFOS[i][2]) {
            surface = IMG_Load(MATERIAL_INFOS[i][2]);
            if (!surface) return false;
            FG_RendererCreateTexture(EX_RENDERER, surface, &EX_MATERIALS[i].normal);
            if (!EX_MATERIALS[i].normal) return false;
            SDL_DestroySurface(surface);
        }
    }
#endif /* USE_MATERIALS */
    if (EX_CAMERA_COUNT) {
        EX_CAMERAS = SDL_calloc(EX_CAMERA_COUNT, sizeof(*EX_CAMERAS));
        if (!EX_CAMERAS) return false;
        for (i = 0; i != EX_CAMERA_COUNT; ++i) EX_CAMERAS[i] = EX_DEF_CAMERA;
    }
    if (EX_QUAD3_COUNT) {
        EX_QUAD3S = SDL_calloc(EX_QUAD3_COUNT, sizeof(*EX_QUAD3S));
        if (!EX_QUAD3S) return false;
        for (i = 0; i != EX_QUAD3_COUNT; ++i) EX_QUAD3S[i] = EX_DEF_QUAD3;
    }
    if (EX_AMBIENT_COUNT) {
        EX_AMBIENTS = SDL_calloc(EX_AMBIENT_COUNT, sizeof(*EX_AMBIENTS));
        if (!EX_AMBIENTS) return false;
        for (i = 0; i != EX_AMBIENT_COUNT; ++i) EX_AMBIENTS[i] = EX_DEF_AMBIENT;
    }
    if (EX_OMNI_COUNT) {
        EX_OMNIS = SDL_calloc(EX_OMNI_COUNT, sizeof(*EX_OMNIS));
        if (!EX_OMNIS) return false;
        for (i = 0; i != EX_OMNI_COUNT; ++i) EX_OMNIS[i] = EX_DEF_OMNI;
    }
    EX_KEYS = SDL_GetKeyboardState(NULL);
    return true;
}

bool EX_PollQuit(void)
{
    SDL_Event event = { .type = SDL_EVENT_FIRST };
    while (SDL_PollEvent(&event)) if (event.type == SDL_EVENT_QUIT) return true;
    return false;
}

#define EX_Input(x) EX_KEYS[SDL_SCANCODE_##x]

#define EX_Inputs(x, y) (float)(EX_KEYS[SDL_SCANCODE_##x] - EX_KEYS[SDL_SCANCODE_##y])

#define EX_SpawnT(T, U)                                                      \
FG_##T *EX_Spawn##T(void);                                                   \
FG_##T *EX_Spawn##T(void)                                                    \
{                                                                            \
    Uint32 i = EX_##U##_COUNT++;                                             \
    EX_##U##S = SDL_realloc(EX_##U##S, EX_##U##_COUNT * sizeof(*EX_##U##S)); \
    if (!EX_##U##S) return NULL;                                             \
    EX_##U##S[i] = EX_DEF_##U;                                               \
    return EX_##U##S + i;                                                    \
}

EX_SpawnT(Camera, CAMERA)
EX_SpawnT(Quad3, QUAD3)
EX_SpawnT(AmbientLight, AMBIENT)
EX_SpawnT(OmniLight, OMNI)

#define EX_MoveCamera(i, ms, rs, mr, ml, mu, md, mb, mf, rl, rr)                  \
do {                                                                              \
    EX_CAMERAS[i].transform.translation.x += (ms) * EX_Inputs(mr, ml) * EX_DELTA; \
    EX_CAMERAS[i].transform.translation.y += (ms) * EX_Inputs(mu, md) * EX_DELTA; \
    EX_CAMERAS[i].transform.translation.z += (ms) * EX_Inputs(mb, mf) * EX_DELTA; \
    EX_CAMERAS[i].transform.rotation      += (rs) * EX_Inputs(rl, rr) * EX_DELTA; \
} while (0)

bool EX_Update(void)
{
    static Uint64 last;
    Uint64        now  = 0;
    if (!FG_RendererDraw(
        EX_RENDERER,
        &(FG_RendererDrawInfo){
            .cameras      = EX_CAMERAS,
            .camera_count = EX_CAMERA_COUNT,
            .quad3_info = {
                .instances = EX_QUAD3S,
                .count     = EX_QUAD3_COUNT
            },
            .shading_info = {
                .ambients      = EX_AMBIENTS,
                .ambient_count = EX_AMBIENT_COUNT,
                .omnis         = EX_OMNIS,
                .omni_count    = EX_OMNI_COUNT
            }
        }
    )) {
        return false;
    }
    now      = SDL_GetTicks();
    EX_DELTA = (float)(now - last);
    last     = now;
    SDL_Log("DELTA: %.0f\n", (double)EX_DELTA);
    return true;
}

bool EX_Quit(void)
{
#ifdef USE_MATERIALS
    Uint32 i = 0;
#endif /* USE_MATERIALS */
    SDL_free(EX_OMNIS);
    SDL_free(EX_AMBIENTS);
    SDL_free(EX_QUAD3S);
    SDL_free(EX_CAMERAS);
#ifdef USE_MATERIALS
    for (i = 0; i != SDL_arraysize(MATERIAL_INFOS); ++i) {
        FG_RendererDestroyTexture(EX_RENDERER, EX_MATERIALS[i].normal);
        FG_RendererDestroyTexture(EX_RENDERER, EX_MATERIALS[i].specular);
        FG_RendererDestroyTexture(EX_RENDERER, EX_MATERIALS[i].albedo);
    }
#endif /* USE_MATERIALS */
    if (!FG_DestroyRenderer(EX_RENDERER)) return false;
    SDL_DestroyWindow(EX_WINDOW);
    SDL_Quit();
    return true;
}

#endif /* EXAMPLES_COMMON_H */
