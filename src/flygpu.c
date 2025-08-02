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
#include "quad3stage.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <stddef.h>

struct FG_Renderer
{
    SDL_Window                      *window;
    SDL_GPUDevice                   *device;
    SDL_GPUTransferBufferCreateInfo  transbuf_info;
    Uint32                           padding0;
    SDL_GPUTransferBuffer           *transbuf;
    SDL_GPUTextureCreateInfo         depthtex_info;
    Uint32                           padding1;
    SDL_GPUDepthStencilTargetInfo    depthtarg_info;
    SDL_GPUViewport                  viewport;
    FG_Quad3Stage                   *quad3stage;
    SDL_GPUFence                    *fence;
};

Sint32 FG_CompareCameras(const void *lhs, const void *rhs);

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

    if (!SDL_SetGPUSwapchainParameters(
        self->device,
        self->window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR,
        vsync ? SDL_GPU_PRESENTMODE_VSYNC : SDL_GPU_PRESENTMODE_IMMEDIATE
    )) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    self->depthtex_info.format               = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    self->depthtex_info.usage                = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    self->depthtex_info.layer_count_or_depth = 1;
    self->depthtex_info.num_levels           = 1;

    self->depthtarg_info.clear_depth = 1.0F;
    self->depthtarg_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    self->depthtarg_info.store_op    = SDL_GPU_STOREOP_DONT_CARE;

    self->viewport.max_depth = 1.0F;

    self->quad3stage = FG_CreateQuad3Stage(
        self->device, SDL_GetGPUSwapchainTextureFormat(self->device, self->window));
    if (!self->quad3stage) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    return self;
}

bool FG_RendererCreateTexture(FG_Renderer        *self,
                              const SDL_Surface  *surface,
                              SDL_GPUTexture    **texture)
{
    Sint32                size     = surface->w * surface->h * 4;
    void                 *transmem = NULL;
    SDL_GPUCommandBuffer *cmdbuf   = NULL;
    SDL_GPUCopyPass      *cpypass  = NULL;

    *texture = NULL;

    if (size <= 0) {
        SDL_SetError("FlyGPU: Invalid surface size!");
        return true;
    }

    if (surface->format != SDL_PIXELFORMAT_ABGR8888) {
        SDL_SetError("FlyGPU: Surface format must be ABGR8888!");
        return true;
    }

    *texture = SDL_CreateGPUTexture(
        self->device,
        &(SDL_GPUTextureCreateInfo){
            .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
            .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width                = (Uint32)surface->w,
            .height               = (Uint32)surface->h,
            .layer_count_or_depth = 1,
            .num_levels           = 1
        }
    );
    if (!texture) return false;

    if (self->transbuf_info.size < (Uint32)size) {
        SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
        self->transbuf_info.size = (Uint32)size;
        self->transbuf           = SDL_CreateGPUTransferBuffer(
            self->device, &self->transbuf_info);
        if (!self->transbuf) return false;
    }

    if (self->fence) {
        if (!SDL_WaitForGPUFences(self->device, true, &self->fence, 1)) return false;
        SDL_ReleaseGPUFence(self->device, self->fence);
        self->fence = NULL;
    }

    transmem = SDL_MapGPUTransferBuffer(self->device, self->transbuf, false);
    if (!transmem) return false;

    SDL_memcpy(transmem, surface->pixels, (size_t)size);

    SDL_UnmapGPUTransferBuffer(self->device, self->transbuf);

    cmdbuf = SDL_AcquireGPUCommandBuffer(self->device);
    if (!cmdbuf) return false;

    cpypass = SDL_BeginGPUCopyPass(cmdbuf);
    SDL_UploadToGPUTexture(
        cpypass,
        &(SDL_GPUTextureTransferInfo){ .transfer_buffer = self->transbuf },
        &(SDL_GPUTextureRegion){
            .texture = *texture,
            .w       = (Uint32)surface->w,
            .h       = (Uint32)surface->h
        },
        false
    );
    SDL_EndGPUCopyPass(cpypass);

    self->fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);
    return self->fence;
}

Sint32 FG_CompareCameras(const void *lhs, const void *rhs)
{
    return (*(FG_Camera *const*)lhs)->priority - (*(FG_Camera *const*)rhs)->priority;
}

bool FG_RendererDraw(FG_Renderer *self, const FG_RendererDrawInfo *info)
{
    const FG_Camera        *cameras[info->camera_count];
    SDL_GPUCommandBuffer   *cmdbuf                      = SDL_AcquireGPUCommandBuffer(self->device);
    SDL_GPUColorTargetInfo  colortarg_info              = { .load_op = SDL_GPU_LOADOP_CLEAR };
    Uint32                  width                       = 0;
    Uint32                  height                      = 0;
    SDL_GPURenderPass      *rndrpass                    = NULL;
    size_t                  i                           = 0;
    FG_Mat4                 projmat                     = { .data = { 0.0F } };
    FG_Mat4                 viewmat                     = { .data = { 0.0F } };
    FG_Mat4                 vpmat                       = { .data = { 0.0F } };
    SDL_GPUCopyPass        *cpypass                     = NULL;

    for (i = 0; i != info->camera_count; ++i) cameras[i] = info->cameras + i;

    if (!cmdbuf) return false;

    if (!SDL_AcquireGPUSwapchainTexture(
        cmdbuf, self->window, &colortarg_info.texture, &width, &height)) {
        return false;
    }

    if (!colortarg_info.texture) return SDL_CancelGPUCommandBuffer(cmdbuf);

    if (self->depthtex_info.width != width || self->depthtex_info.height != height) {
        SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
        self->depthtex_info.width    = width;
        self->depthtex_info.height   = height;
        self->depthtarg_info.texture = SDL_CreateGPUTexture(
            self->device, &self->depthtex_info);
        if (!self->depthtarg_info.texture) return false;
    }

    rndrpass = SDL_BeginGPURenderPass(
        cmdbuf, &colortarg_info, 1, &self->depthtarg_info);
    SDL_EndGPURenderPass(rndrpass);

    SDL_qsort(cameras, info->camera_count, sizeof(*cameras), FG_CompareCameras);

    colortarg_info.load_op = SDL_GPU_LOADOP_LOAD;

    for (i = 0; i != info->camera_count; ++i) {
        self->viewport.x = (float)width * cameras[i]->viewport.tl.x;
        self->viewport.y = (float)height * cameras[i]->viewport.tl.y;
        self->viewport.w = (float)width
                         * (cameras[i]->viewport.br.x - cameras[i]->viewport.tl.x);
        self->viewport.h = (float)height
                         * (cameras[i]->viewport.br.y - cameras[i]->viewport.tl.y);

        FG_SetProjMat4(
            &cameras[i]->perspective, self->viewport.w / self->viewport.h, &projmat);
        FG_SetViewMat4(&cameras[i]->transform, &viewmat);
        FG_MulMat4s(&projmat, &viewmat, &vpmat);

        cpypass = SDL_BeginGPUCopyPass(cmdbuf);
        if (!FG_Quad3StageCopy(
            self->quad3stage,
            cpypass,
            cameras[i]->transform.translation.z,
            &vpmat,
            &info->quad3s_info
        )) {
            return false;
        }
        SDL_EndGPUCopyPass(cpypass);

        rndrpass = SDL_BeginGPURenderPass(
            cmdbuf, &colortarg_info, 1, &self->depthtarg_info);
        SDL_SetGPUViewport(rndrpass, &self->viewport);
        FG_Quad3StageDraw(self->quad3stage, rndrpass);
        SDL_EndGPURenderPass(rndrpass);
    }

    if (self->fence) {
        if (!SDL_WaitForGPUFences(self->device, true, &self->fence, 1)) return false;
        SDL_ReleaseGPUFence(self->device, self->fence);
        self->fence = NULL;
    }

    return SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void FG_RendererDestroyTexture(FG_Renderer *self, SDL_GPUTexture *texture)
{
    SDL_ReleaseGPUTexture(self->device, texture);
}

void FG_DestroyRenderer(FG_Renderer *self)
{
    if (!self) return;
    if (self->device) {
        SDL_ReleaseGPUFence(self->device, self->fence);
        FG_DestroyQuad3Stage(self->quad3stage);
        SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
        SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
        SDL_WaitForGPUIdle(self->device);
        SDL_ReleaseWindowFromGPUDevice(self->device, self->window);
        SDL_DestroyGPUDevice(self->device);
    }
    SDL_free(self);
}
