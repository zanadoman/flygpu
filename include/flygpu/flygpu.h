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

#ifndef FLYGPU_FLYGPU_H
#define FLYGPU_FLYGPU_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "linalg.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>

typedef struct
{
    FG_Vec2 tl;
    FG_Vec2 br;
} FG_Rect;

typedef struct
{
    FG_Rect        viewport;
    FG_Perspective perspective;
    FG_Transform3  transform;
    Sint32         priority;
    Uint32         mask;
} FG_Camera;

typedef struct
{
    FG_Vec3 tl;
    FG_Vec3 bl;
    FG_Vec3 br;
    FG_Vec3 tr;
} FG_QuadColor;

typedef struct
{
    SDL_GPUTexture *albedo;
    SDL_GPUTexture *specular;
    SDL_GPUTexture *normal;
} FG_Material;

typedef struct
{
    FG_Transform3      transform;
    FG_QuadColor       color;
    const FG_Material *material;
    FG_Rect            coords;
    Uint32             mask;
    Uint8              padding0[4];
} FG_Quad3;

typedef struct
{
    const FG_Quad3 *instances;
    Uint32          count;
    Uint8           padding0[4];
} FG_Quad3StageDrawInfo;

typedef struct
{
    FG_Vec3 direction;
    Uint8   padding0[4];
    FG_Vec3 color;
    Uint32  mask;
} FG_AmbientLight;

typedef struct
{
    FG_Vec3 translation;
    float   radius;
    FG_Vec3 color;
    Uint32  mask;
} FG_OmniLight;

typedef struct
{
    const FG_AmbientLight *ambients;
    Uint32                 ambient_count;
    Uint8                  padding0[4];
    const FG_OmniLight    *omnis;
    Uint32                 omni_count;
    Uint8                  padding1[4];
} FG_ShadingStageDrawInfo;

typedef struct
{
    const FG_Camera         *cameras;
    Uint32                   camera_count;
    Uint8                    padding0[4];
    FG_Quad3StageDrawInfo    quad3_info;
    FG_ShadingStageDrawInfo  shading_info;
} FG_RendererDrawInfo;

typedef struct FG_Renderer FG_Renderer;

FG_Renderer *FG_CreateRenderer(SDL_Window *window, bool vsync);

bool FG_RendererCreateTexture(FG_Renderer        *self,
                              const SDL_Surface  *surface,
                              SDL_GPUTexture    **texture);

bool FG_RendererDraw(FG_Renderer *self, const FG_RendererDrawInfo *info);

void FG_RendererDestroyTexture(FG_Renderer *self, SDL_GPUTexture *texture);

bool FG_DestroyRenderer(FG_Renderer *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FLYGPU_FLYGPU_H */
