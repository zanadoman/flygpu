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

#include "linalg.h" /* IWYU pragma: export */

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    FG_Vec4 bl;
    FG_Vec4 br;
    FG_Vec4 tr;
    FG_Vec4 tl;
} FG_QuadColor;

typedef struct
{
    FG_Transform3 transform;
    FG_QuadColor  color;
} FG_Quad3;

typedef struct
{
    const FG_Quad3 *insts;
    Uint32          count;
    Uint32          padding0;
} FG_RendererQuad3sDrawInfo;

typedef struct
{
    FG_RendererQuad3sDrawInfo quad3s_info;
} FG_RendererDrawInfo;

typedef struct FG_Renderer FG_Renderer;

FG_Renderer *FG_CreateRenderer(SDL_Window *window, bool vsync);

SDL_GPUTexture *FG_RendererUploadSurface(FG_Renderer *self, const SDL_Surface *surface);

bool FG_RendererDraw(FG_Renderer *self, const FG_RendererDrawInfo *info);

void FG_DestroyRenderer(FG_Renderer *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FLYGPU_FLYGPU_H */
