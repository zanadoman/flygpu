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

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>

#include <SDL3/SDL_begin_code.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    float x;
    float y;
} FG_Vec2;

typedef struct
{
    FG_Vec2 tl;
    FG_Vec2 br;
} FG_Rect;

typedef struct
{
    float fov;
    float near;
    float far;
} FG_Perspective;

typedef struct
{
    float x;
    float y;
    float z;
} FG_Vec3;

typedef struct
{
    FG_Vec3 transl;
    float   rotation;
    FG_Vec2 scale;
} FG_Transform3;

typedef struct
{
    Sint32         priority;
    FG_Rect        viewport;
    FG_Perspective perspective;
    FG_Vec3        ambient;
    FG_Transform3  transf;
    Uint32         mask;
} FG_Camera;

typedef union
{
    struct
    {
        SDL_GPUTexture *albedo;
        SDL_GPUTexture *specular;
        SDL_GPUTexture *normal;
    }               maps;
    SDL_GPUTexture *iter[3];
} FG_Material;

typedef struct
{
    FG_Vec3 tl;
    FG_Vec3 bl;
    FG_Vec3 br;
    FG_Vec3 tr;
} FG_QuadColor;

typedef struct
{
    FG_Transform3      transf;
    const FG_Material *material;
    FG_QuadColor       color;
    FG_Rect            coords;
    Uint32             mask;
    Uint32             padding;
} FG_Quad3;

typedef struct
{
    Uint32          count;
    Uint32          padding;
    const FG_Quad3 *quad3s;
} FG_Quad3StageDrawInfo;

typedef struct
{
    FG_Vec3 direction;
    Uint32  padding;
    FG_Vec3 color;
    Uint32  mask;
} FG_DirectLight;

typedef struct
{
    FG_Vec3 transl;
    float   radius;
    FG_Vec3 color;
    Uint32  mask;
} FG_OmniLight;

typedef struct
{
    Uint32                direct_count;
    Uint32                omni_count;
    const FG_DirectLight *directs;
    const FG_OmniLight   *omnis;
} FG_ShadingStageDrawInfo;

typedef struct
{
    Uint32                   camera_count;
    Uint32                   padding;
    const FG_Camera         *cameras;
    FG_Quad3StageDrawInfo    quad3_info;
    FG_ShadingStageDrawInfo  shading_info;
} FG_RendererDrawInfo;

typedef struct FG_Renderer FG_Renderer;

SDL_DECLSPEC FG_Renderer * SDLCALL FG_CreateRenderer(SDL_Window *window,
                                                     bool        vsync,
                                                     bool        debug);

SDL_DECLSPEC bool SDLCALL FG_RendererCreateTexture(FG_Renderer        *self,
                                                   const SDL_Surface  *surface,
                                                   bool                mipmaps,
                                                   SDL_GPUTexture    **texture);

SDL_DECLSPEC bool SDLCALL FG_RendererDraw(FG_Renderer               *self,
                                          const FG_RendererDrawInfo *info);

SDL_DECLSPEC void SDLCALL FG_RendererDestroyTexture(FG_Renderer    *self,
                                                    SDL_GPUTexture *texture);

SDL_DECLSPEC void SDLCALL FG_DestroyRenderer(FG_Renderer *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#include <SDL3/SDL_close_code.h>

#endif /* FLYGPU_FLYGPU_H */
