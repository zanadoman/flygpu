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

#include "../include/flygpu/linalg.h"
#include "quad3pline.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

struct FG_Renderer
{
    SDL_Window             *window;
    SDL_GPUDevice          *device;
    SDL_GPUColorTargetInfo  target;
    FG_Quad3Pline          *quad3pline;
};

FG_Renderer *FG_CreateRenderer(SDL_Window *window, bool vsync)
{
    FG_Renderer *self = SDL_calloc(1, sizeof(*self));

    if (!self) return NULL;

    self->window = window;

    self->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
    if (!self->device) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    if (!SDL_ClaimWindowForGPUDevice(self->device, self->window)) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    if (!SDL_SetGPUSwapchainParameters(self->device, self->window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR, vsync ? SDL_GPU_PRESENTMODE_VSYNC
                                                    : SDL_GPU_PRESENTMODE_IMMEDIATE))
    {
        FG_DestroyRenderer(self);
        return NULL;
    }

    self->target.load_op = SDL_GPU_LOADOP_CLEAR;

    self->quad3pline = FG_CreateQuad3Pline(self->device, self->window);
    if (!self->quad3pline) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    return self;
}

bool FG_RendererDraw(FG_Renderer *self, const FG_Quad3 *begin, const FG_Quad3 *end)
{
    SDL_GPUCommandBuffer *cmdbuf      = SDL_AcquireGPUCommandBuffer(self->device);
    Uint32 width                      = 0;
    Uint32 height                     = 0;
    SDL_GPUCopyPass      *copy_pass   = NULL;
    FG_Mat4               projmat;
    SDL_GPURenderPass    *render_pass = NULL;

    if (!cmdbuf) return false;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            cmdbuf, self->window, &self->target.texture, &width, &height)) {
        return SDL_CancelGPUCommandBuffer(cmdbuf);
    }

    if (self->target.texture) {
        copy_pass = SDL_BeginGPUCopyPass(cmdbuf);
        FG_SetProjMat4(FG_DegToRad(60.0F), (float)width / (float)height, &projmat);
        FG_Quad3PlineCopy(self->quad3pline, copy_pass, &projmat, begin, end);
        SDL_EndGPUCopyPass(copy_pass);
        render_pass = SDL_BeginGPURenderPass(cmdbuf, &self->target, 1, NULL);
        FG_Quad3PlineDraw(self->quad3pline, render_pass);
        SDL_EndGPURenderPass(render_pass);
    }

    return SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void FG_DestroyRenderer(FG_Renderer *self)
{
    if (!self) return;
    FG_ReleaseQuad3Pline(self->quad3pline);
    SDL_ReleaseWindowFromGPUDevice(self->device, self->window);
    SDL_DestroyGPUDevice(self->device);
    SDL_free(self);
}
