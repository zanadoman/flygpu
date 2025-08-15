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

#include "shadingstage.h"

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stdbool.h>
#include <stddef.h>

#define FG_LIGHT_VARIANTS 2

struct FG_ShadingStage
{
    SDL_GPUDevice                 *device;
    SDL_GPUShader                 *vertspv;
    SDL_GPUShader                 *fragspv;
    Uint32                         capacity;
    Uint8                          padding0[4];
    const void                   **lights;
    SDL_GPUTextureSamplerBinding   sampler_binds[FG_GBUF_COUNT];
    SDL_GPUBufferCreateInfo        ssbo_infos[FG_LIGHT_VARIANTS];
    SDL_GPUBuffer                 *ssbos[FG_LIGHT_VARIANTS];
    SDL_GPUTransferBuffer         *transbufs[FG_LIGHT_VARIANTS];
    struct
    {
        FG_Vec3 origo;
        Uint32  counts[FG_LIGHT_VARIANTS];
    }                              ubo;
    Uint8                          padding1[4];
    SDL_GPUGraphicsPipeline       *pipeline;
};

static bool FG_FilterAmbientLight(Uint32 mask, const void *light);

static bool FG_FilterOmniLight(Uint32 mask, const void *light);

static bool FG_ShadingStageSubCopy(FG_ShadingStage  *self,
                                   SDL_GPUCopyPass  *cpypass,
                                   Uint8             dst,
                                   const void       *src,
                                   Uint32            count,
                                   Uint8             size,
                                   Uint32            mask,
                                   bool            (*filter)(Uint32, const void *));

FG_ShadingStage *FG_CreateShadingStage(SDL_GPUDevice        *device,
                                       SDL_GPUTextureFormat  targbuf_fmt)
{
    FG_ShadingStage                   *self = SDL_calloc(1, sizeof(*self));
    Uint8                              i    = 0;
    SDL_GPUGraphicsPipelineCreateInfo  info = {
        .target_info = {
            .color_target_descriptions = &(SDL_GPUColorTargetDescription){
                .format = targbuf_fmt
            },
            .num_color_targets         = 1
        }
    };

    if (!self) return NULL;

    self->device = device;

    self->vertspv = FG_LoadShader(
        self->device,
        "./shaders/viewport.vert.spv",
        SDL_GPU_SHADERSTAGE_VERTEX,
        0,
        0,
        0
    );
    if (!self->vertspv) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    self->fragspv = FG_LoadShader(
        self->device,
        "./shaders/shading.frag.spv",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        SDL_arraysize(self->sampler_binds),
        SDL_arraysize(self->ssbos),
        1
    );
    if (!self->fragspv) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        self->sampler_binds[i].sampler = SDL_CreateGPUSampler(
            self->device, &(SDL_GPUSamplerCreateInfo){ .props = 0 });
        if (!self->sampler_binds[i].sampler) {
            FG_DestroyShadingStage(self);
            return NULL;
        }
    }

    for (i = 0; i != FG_LIGHT_VARIANTS; ++i) {
        self->ssbo_infos[i].usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        self->ssbo_infos[i].size  = 1;

        self->ssbos[i] = SDL_CreateGPUBuffer(self->device, self->ssbo_infos + i);
        if (!self->ssbos[i]) {
            FG_DestroyShadingStage(self);
            return NULL;
        }

        self->transbufs[i] = SDL_CreateGPUTransferBuffer(
            self->device,
            &(SDL_GPUTransferBufferCreateInfo){ .size = self->ssbo_infos[i].size }
        );
        if (!self->transbufs[i]) {
            FG_DestroyShadingStage(self);
            return NULL;
        }
    }

    info.vertex_shader   = self->vertspv;
    info.fragment_shader = self->fragspv;

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    if (!self->pipeline) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    return self;
}

void FG_ShadingStageUpdate(FG_ShadingStage        *self,
                           SDL_GPUColorTargetInfo *gbuftarg_infos)
{
    Uint8 i = 0;

    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        self->sampler_binds[i].texture = gbuftarg_infos[i].texture;
    }
}

bool FG_FilterAmbientLight(Uint32 mask, const void *light) {
    const FG_AmbientLight *ambient = (const FG_AmbientLight *)light;

    return ambient->mask & mask && ambient->direction.z < 0.0F;
}

bool FG_FilterOmniLight(Uint32 mask, const void *light) {
    const FG_OmniLight *omni = (const FG_OmniLight *)light;

    return omni->mask & mask && 0.0F < omni->radius;
}

bool FG_ShadingStageSubCopy(FG_ShadingStage  *self,
                            SDL_GPUCopyPass  *cpypass,
                            Uint8             dst,
                            const void       *src,
                            Uint32            count,
                            Uint8             size,
                            Uint32            mask,
                            bool            (*filter)(Uint32, const void *))
{
    const Uint8 *it        = src;
    const void  *end       = it + count * size;
    Uint32       ssbo_size = 0;
    Uint8       *transmem  = NULL;
    Uint32       i         = 0;

    if (self->capacity < count) {
        self->capacity = count;

        self->lights = SDL_realloc(
            self->lights, self->capacity * sizeof(*self->lights));
        if (!self->lights) return false;
    }

    for (it = src, self->ubo.counts[dst] = 0; it != end; it += size) {
        if (filter(mask, it)) self->lights[self->ubo.counts[dst]++] = it;
    }

    if (!self->ubo.counts[dst]) return true;

    ssbo_size = self->ubo.counts[dst] * size;

    if (self->ssbo_infos[dst].size < ssbo_size) {
        SDL_ReleaseGPUBuffer(self->device, self->ssbos[dst]);
        self->ssbo_infos[dst].size = ssbo_size;
        self->ssbos[dst]           = SDL_CreateGPUBuffer(
            self->device, &self->ssbo_infos[dst]);
        if (!self->ssbos[dst]) return false;

        SDL_ReleaseGPUTransferBuffer(self->device, self->transbufs[dst]);
        self->transbufs[dst] = SDL_CreateGPUTransferBuffer(
            self->device,
            &(SDL_GPUTransferBufferCreateInfo){ .size = self->ssbo_infos[dst].size }
        );
        if (!self->transbufs[dst]) return false;
    }

    transmem = SDL_MapGPUTransferBuffer(self->device, self->transbufs[dst], true);
    if (!transmem) return false;

    for (i = 0; i != self->ubo.counts[dst]; ++i, transmem += size) {
        SDL_memcpy(transmem, self->lights[i], size);
    }

    SDL_UnmapGPUTransferBuffer(self->device, self->transbufs[dst]);

    SDL_UploadToGPUBuffer(
        cpypass,
        &(SDL_GPUTransferBufferLocation){ .transfer_buffer = self->transbufs[dst] },
        &(SDL_GPUBufferRegion){
            .buffer = self->ssbos[dst],
            .size   = self->ssbo_infos[dst].size
        },
        false
    );

    return true;
}

bool FG_ShadingStageCopy(FG_ShadingStage               *self,
                         SDL_GPUCopyPass               *cpypass,
                         Uint32                         mask,
                         const FG_ShadingStageDrawInfo *info)
{
    return FG_ShadingStageSubCopy(
        self,
        cpypass,
        0,
        info->ambients,
        info->ambient_count,
        sizeof(*info->ambients),
        mask,
        FG_FilterAmbientLight
    ) &&
    FG_ShadingStageSubCopy(
        self,
        cpypass,
        1,
        info->omnis,
        info->omni_count,
        sizeof(*info->omnis),
        mask,
        FG_FilterOmniLight
    );
}

void FG_ShadingStageDraw(FG_ShadingStage      *self,
                         SDL_GPUCommandBuffer *cmdbuf,
                         SDL_GPURenderPass    *rndrpass,
                         const FG_Vec3        *origo)
{
    self->ubo.origo = *origo;

    SDL_BindGPUFragmentSamplers(
        rndrpass, 0, self->sampler_binds, SDL_arraysize(self->sampler_binds));
    SDL_BindGPUFragmentStorageBuffers(
        rndrpass, 0, self->ssbos, SDL_arraysize(self->ssbos));
    SDL_PushGPUFragmentUniformData(cmdbuf, 0, &self->ubo, sizeof(self->ubo));
    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_DrawGPUPrimitives(rndrpass, 3, 1, 0, 0);
}

void FG_DestroyShadingStage(FG_ShadingStage *self)
{
    Uint8 i = 0;

    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    for (i = 0; i != FG_LIGHT_VARIANTS; ++i) {
        SDL_ReleaseGPUTransferBuffer(self->device, self->transbufs[i]);
        SDL_ReleaseGPUBuffer(self->device, self->ssbos[i]);
    }
    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        SDL_ReleaseGPUSampler(self->device, self->sampler_binds[i].sampler);
    }
    SDL_free(self->lights);
    SDL_ReleaseGPUShader(self->device, self->fragspv);
    SDL_ReleaseGPUShader(self->device, self->vertspv);
    SDL_free(self);
}
