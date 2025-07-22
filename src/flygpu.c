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

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

struct FG_Renderer
{
    SDL_Window                    *window;
    SDL_GPUDevice                 *device;
    SDL_GPUColorTargetInfo         colortarg_info;
    SDL_GPUTextureCreateInfo       depthtex_info;
    Uint32                         padding0;
    SDL_GPUDepthStencilTargetInfo  depthtarg_info;
    FG_Quad3Pline                 *quad3pline;
    SDL_GPUFence                  *cmdbuf_fence;
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

    self->colortarg_info.load_op = SDL_GPU_LOADOP_CLEAR;

    self->depthtex_info.format               = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    self->depthtex_info.usage                = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    self->depthtex_info.layer_count_or_depth = 1;
    self->depthtex_info.num_levels           = 1;

    self->depthtarg_info.clear_depth = 1.0F;
    self->depthtarg_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    self->depthtarg_info.store_op    = SDL_GPU_STOREOP_DONT_CARE;

    self->quad3pline = FG_CreateQuad3Pline(self->device, self->window);
    if (!self->quad3pline) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    return self;
}

bool FG_RendererDraw(FG_Renderer *self, const FG_Quad3 *begin, const FG_Quad3 *end)
{
    SDL_GPUCommandBuffer *cmdbuf   = SDL_AcquireGPUCommandBuffer(self->device);
    Uint32 width                   = 0;
    Uint32 height                  = 0;
    SDL_GPUCopyPass      *cpypass  = NULL;
    FG_Mat4               projmat;
    SDL_GPURenderPass    *rndrpass = NULL;

    if (!cmdbuf) return false;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            cmdbuf, self->window, &self->colortarg_info.texture, &width, &height))
    {
        return SDL_CancelGPUCommandBuffer(cmdbuf);
    }

    if (width && height && (self->depthtex_info.width != width || self->depthtex_info.height != height))
    {
        SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
        self->depthtex_info.width    = width;
        self->depthtex_info.height   = height;
        self->depthtarg_info.texture = SDL_CreateGPUTexture(self->device, &self->depthtex_info);
    }

    if (self->colortarg_info.texture && self->depthtarg_info.texture) {
        cpypass = SDL_BeginGPUCopyPass(cmdbuf);
        FG_SetProjMat4(FG_DegToRad(60.0F), (float)width / (float)height, &projmat);
        FG_Quad3PlineCopy(self->quad3pline, cpypass, &projmat, begin, end);
        SDL_EndGPUCopyPass(cpypass);
        rndrpass = SDL_BeginGPURenderPass(cmdbuf, &self->colortarg_info, 1, &self->depthtarg_info);
        FG_Quad3PlineDraw(self->quad3pline, rndrpass);
        SDL_EndGPURenderPass(rndrpass);
    }

    if (self->cmdbuf_fence) {
        SDL_WaitForGPUFences(self->device, true, &self->cmdbuf_fence, 1);
        SDL_ReleaseGPUFence(self->device, self->cmdbuf_fence);
    }
    self->cmdbuf_fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);

    return self->cmdbuf_fence;
}

void FG_DestroyRenderer(FG_Renderer *self)
{
    if (!self) return;
    SDL_ReleaseGPUFence(self->device, self->cmdbuf_fence);
    SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
    FG_ReleaseQuad3Pline(self->quad3pline);
    SDL_ReleaseWindowFromGPUDevice(self->device, self->window);
    SDL_DestroyGPUDevice(self->device);
    SDL_free(self);
}
