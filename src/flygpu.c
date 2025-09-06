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

#include "quad3stage.h"
#include "linalg.h"
#include "shader.h"
#include "shadingstage.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <stdbool.h>
#include <stddef.h>

struct FG_Renderer
{
    SDL_Window                      *window;
    SDL_GPUDevice                   *device;
    FG_Material                      material;
    SDL_GPUTransferBufferCreateInfo  transbuf_info;
    Uint8                            padding0[4];
    SDL_GPUTransferBuffer           *transbuf;
    SDL_GPUTextureCreateInfo         targbuf_info;
    Uint8                            padding1[4];
    SDL_GPUColorTargetInfo           gbuftarg_infos[FG_GBUF_COUNT];
    SDL_GPUDepthStencilTargetInfo    depthtarg_info;
    FG_Quad3Stage                   *quad3stage;
    FG_ShadingStage                 *shadingstage;
    SDL_GPUFence                    *fence;
};

static Sint32 FG_CompareCameras(const void *lhs, const void *rhs);

FG_Renderer *FG_CreateRenderer(SDL_Window *window, bool vsync)
{
    FG_Renderer *self    = SDL_calloc(1, sizeof(*self));
    SDL_Surface  surface = {
        .format = SDL_PIXELFORMAT_ABGR8888,
        .w      = 1,
        .h      = 1
    };
    Uint8        i       = 0;

    if (!self) return NULL;

    self->window = window;

    self->device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL, true, NULL);
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
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        vsync ? SDL_GPU_PRESENTMODE_VSYNC : SDL_GPU_PRESENTMODE_IMMEDIATE
    )) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    surface.pixels = &(Uint32){ 0xFFFFFFFF };

    FG_RendererCreateTexture(self, &surface, &self->material.albedo);
    if (!self->material.albedo) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    surface.pixels = &(Uint32){ 0x000A0A0A };

    FG_RendererCreateTexture(self, &surface, &self->material.specular);
    if (!self->material.specular) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    surface.pixels = &(Uint32){ 0x00FF8080 };

    FG_RendererCreateTexture(self, &surface, &self->material.normal);
    if (!self->material.normal) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    self->targbuf_info.layer_count_or_depth = 1;
    self->targbuf_info.num_levels           = 1;

    for (i = 0; i != SDL_arraysize(self->gbuftarg_infos); ++i) {
        self->gbuftarg_infos[i].load_op = SDL_GPU_LOADOP_CLEAR;
    }

    self->depthtarg_info.clear_depth      = 1.0F;
    self->depthtarg_info.load_op          = SDL_GPU_LOADOP_CLEAR;
    self->depthtarg_info.store_op         = SDL_GPU_STOREOP_DONT_CARE;
    self->depthtarg_info.stencil_load_op  = SDL_GPU_LOADOP_DONT_CARE;
    self->depthtarg_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

    self->quad3stage = FG_CreateQuad3Stage(self->device);
    if (!self->quad3stage) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    self->shadingstage = FG_CreateShadingStage(
        self->device, SDL_GetGPUSwapchainTextureFormat(self->device, self->window));
    if (!self->shadingstage) {
        FG_DestroyRenderer(self);
        return NULL;
    }

    return self;
}

bool FG_RendererCreateTexture(FG_Renderer        *self,
                              const SDL_Surface  *surface,
                              SDL_GPUTexture    **texture)
{
    Sint32                    size     = surface->w * surface->h * 4;
    SDL_GPUTextureCreateInfo  info     = {
        .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER
                              | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width                = (Uint32)surface->w,
        .height               = (Uint32)surface->h,
        .layer_count_or_depth = 1,
        .num_levels           = 1
    };
    void                     *transmem = NULL;
    SDL_GPUCommandBuffer     *cmdbuf   = NULL;
    SDL_GPUCopyPass          *cpypass  = NULL;

    *texture = NULL;

    if (SDL_MUSTLOCK(surface)) {
        SDL_SetError("FlyGPU: Invalid surface!");
        return true;
    }

    if (size <= 0) {
        SDL_SetError("FlyGPU: Invalid surface size!");
        return true;
    }

    if (surface->format != SDL_PIXELFORMAT_ABGR8888) {
        SDL_SetError("FlyGPU: Surface format must be ABGR8888!");
        return true;
    }

    info.num_levels += (Uint32)(SDL_logf((float)SDL_max(info.width, info.height)));

    *texture = SDL_CreateGPUTexture(self->device, &info);
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
            .w       = info.width,
            .h       = info.height,
            .d       = 1
        },
        false
    );
    SDL_EndGPUCopyPass(cpypass);

    if (1 < info.num_levels) SDL_GenerateMipmapsForGPUTexture(cmdbuf, *texture);

    self->fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);
    return self->fence;
}

Sint32 FG_CompareCameras(const void *lhs, const void *rhs)
{
    return (*(FG_Camera *const *)lhs)->priority - (*(FG_Camera *const *)rhs)->priority;
}

bool FG_RendererDraw(FG_Renderer *self, const FG_RendererDrawInfo *info)
{
    Uint32                  i                           = 0;
    const FG_Camera        *cameras[info->camera_count];
    SDL_GPUCommandBuffer   *cmdbuf                      = SDL_AcquireGPUCommandBuffer(self->device);
    SDL_GPUColorTargetInfo  swapctarg_info              = { .texture = NULL };
    Uint32                  width                       = 0;
    Uint32                  height                      = 0;
    SDL_GPURenderPass      *rndrpass                    = NULL;
    SDL_GPUViewport         viewport                    = { .max_depth = 1.0F };
    FG_Mat4                 projmat                     = { { 0.0F } };
    FG_Mat4                 viewmat                     = { { 0.0F } };
    FG_Mat4                 vpmat                       = { { 0.0F } };
    SDL_GPUCopyPass        *cpypass                     = NULL;

    for (i = 0; i != info->camera_count; ++i) cameras[i] = info->cameras + i;

    if (!cmdbuf) return false;

    if (!SDL_AcquireGPUSwapchainTexture(
        cmdbuf, self->window, &swapctarg_info.texture, &width, &height)) {
        return false;
    }

    if (!swapctarg_info.texture) return SDL_CancelGPUCommandBuffer(cmdbuf);

    swapctarg_info.load_op = SDL_GPU_LOADOP_CLEAR;

    rndrpass = SDL_BeginGPURenderPass(cmdbuf, &swapctarg_info, 1, NULL);
    SDL_EndGPURenderPass(rndrpass);

    SDL_qsort(cameras, info->camera_count, sizeof(*cameras), FG_CompareCameras);

    swapctarg_info.load_op = SDL_GPU_LOADOP_LOAD;

    if (self->targbuf_info.width != width || self->targbuf_info.height != height) {
        self->targbuf_info.width  = width;
        self->targbuf_info.height = height;

        self->targbuf_info.format = FG_GBUF_FORMAT;
        self->targbuf_info.usage  = SDL_GPU_TEXTUREUSAGE_SAMPLER
                                  | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

        for (i = 0; i != SDL_arraysize(self->gbuftarg_infos); ++i) {
            SDL_ReleaseGPUTexture(self->device, self->gbuftarg_infos[i].texture);
            self->gbuftarg_infos[i].texture = SDL_CreateGPUTexture(
                self->device, &self->targbuf_info);
            if (!self->gbuftarg_infos[i].texture) return false;
        }

        SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
        self->targbuf_info.format    = FG_DEPTH_FORMAT;
        self->targbuf_info.usage     = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        self->depthtarg_info.texture = SDL_CreateGPUTexture(
            self->device, &self->targbuf_info);
        if (!self->depthtarg_info.texture) return false;

        FG_ShadingStageUpdate(self->shadingstage, self->gbuftarg_infos);
    }

    for (i = 0; i != info->camera_count; ++i) {
        viewport.x = (float)width * cameras[i]->viewport.tl.x;
        viewport.y = (float)height * cameras[i]->viewport.tl.y;
        viewport.w = (float)width
                   * (cameras[i]->viewport.br.x - cameras[i]->viewport.tl.x);
        viewport.h = (float)height
                   * (cameras[i]->viewport.br.y - cameras[i]->viewport.tl.y);

        FG_SetProjMat4(&cameras[i]->perspective, viewport.w / viewport.h, &projmat);
        FG_SetViewMat4(&cameras[i]->transform, &viewmat);
        FG_MulMat4s(&projmat, &viewmat, &vpmat);

        cpypass = SDL_BeginGPUCopyPass(cmdbuf);
        if (!FG_Quad3StageCopy(
            self->quad3stage, cpypass, cameras[i]->mask, &vpmat, &info->quad3_info)) {
            return false;
        }
        if (!FG_ShadingStageCopy(
            self->shadingstage,
            cpypass,
            cameras[i]->mask,
            &info->shading_info
        )) {
            return false;
        }
        SDL_EndGPUCopyPass(cpypass);

        rndrpass = SDL_BeginGPURenderPass(
            cmdbuf,
            self->gbuftarg_infos,
            SDL_arraysize(self->gbuftarg_infos),
            &self->depthtarg_info
        );
        SDL_SetGPUViewport(rndrpass, &viewport);
        FG_Quad3StageDraw(self->quad3stage, rndrpass, &self->material);
        SDL_EndGPURenderPass(rndrpass);

        rndrpass = SDL_BeginGPURenderPass(cmdbuf, &swapctarg_info, 1, NULL);
        SDL_SetGPUScissor(
            rndrpass,
            &(SDL_Rect){
                .x = (Sint32)viewport.x,
                .y = (Sint32)viewport.y,
                .w = (Sint32)viewport.w,
                .h = (Sint32)viewport.h
            }
        );
        FG_ShadingStageDraw(
            self->shadingstage,
            cmdbuf,
            rndrpass,
            &cameras[i]->ambient,
            &cameras[i]->transform.translation
        );
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

bool FG_DestroyRenderer(FG_Renderer *self)
{
    Uint8 i = 0;

    if (!self) return true;
    if (self->device) {
        SDL_ReleaseGPUFence(self->device, self->fence);
        FG_DestroyShadingStage(self->shadingstage);
        FG_DestroyQuad3Stage(self->quad3stage);
        SDL_ReleaseGPUTexture(self->device, self->depthtarg_info.texture);
        for (i = 0; i != SDL_arraysize(self->gbuftarg_infos); ++i) {
            SDL_ReleaseGPUTexture(self->device, self->gbuftarg_infos[i].texture);
        }
        SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
        SDL_ReleaseGPUTexture(self->device, self->material.normal);
        SDL_ReleaseGPUTexture(self->device, self->material.specular);
        SDL_ReleaseGPUTexture(self->device, self->material.albedo);
        if (!SDL_WaitForGPUIdle(self->device)) return false;
        SDL_ReleaseWindowFromGPUDevice(self->device, self->window);
        SDL_DestroyGPUDevice(self->device);
    }
    SDL_free(self);
    return true;
}
