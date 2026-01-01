/* clang-format off */

/*
  FlyGPU
  Copyright (C) 2025-2026 Domán Zana

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

#include "shading_stage.h"

#include "../include/flygpu/flygpu.h"
#include "config.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stdbool.h>
#include <stddef.h>

#define FG_LIGHT_VARIANTS 2

struct FG_ShadingStage
{
    SDL_GPUDevice                 *device;
    SDL_GPUShader                 *vertshdr;
    SDL_GPUShader                 *fragshdr;
    SDL_GPUTextureSamplerBinding   sampler_binds[FG_GBUF_COUNT];
    Uint32                         capacity;
    Uint32                         padding;
    const void                   **lights;
    SDL_GPUBufferCreateInfo        ssbo_infos[FG_LIGHT_VARIANTS];
    SDL_GPUBuffer                 *ssbos[FG_LIGHT_VARIANTS];
    SDL_GPUTransferBuffer         *transbufs[FG_LIGHT_VARIANTS];
    struct
    {
        FG_Vec3 origo;
        Uint32  directs_size;
        FG_Vec3 ambient;
        Uint32  omnis_size;
        Uint32  padding[3];
        float   shine;
    }                              ubo;
    SDL_GPUGraphicsPipeline       *pipeline;
};

typedef bool (SDLCALL *FG_LightFilter)(Uint32 mask, const void *light);

static bool SDLCALL FG_AmbientLightFilter(Uint32 mask, const void *light);

static bool SDLCALL FG_OmniLightFilter(Uint32 mask, const void *light);

static bool FG_ShadingStageSubCopy(FG_ShadingStage *self,
                                   SDL_GPUCopyPass *cpypass,
                                   Uint8            dst,
                                   Uint32          *dst_size,
                                   const void      *src,
                                   Uint32           src_count,
                                   Uint8            size,
                                   Uint32           mask,
                                   FG_LightFilter   filter);

FG_ShadingStage * FG_CreateShadingStage(SDL_GPUDevice        *device,
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

    self->vertshdr = FG_LoadShader(
        self->device, "viewport.vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0);
    if (!self->vertshdr) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    self->fragshdr = FG_LoadShader(
        self->device,
        "shading.frag",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        SDL_arraysize(self->sampler_binds),
        FG_LIGHT_VARIANTS,
        1
    );
    if (!self->fragshdr) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        self->sampler_binds[i].sampler = SDL_CreateGPUSampler(
            self->device,
            &(SDL_GPUSamplerCreateInfo){
                .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
            }
        );
        if (!self->sampler_binds[i].sampler) {
            FG_DestroyShadingStage(self);
            return NULL;
        }
    }

    for (i = 0; i != FG_LIGHT_VARIANTS; ++i) {
        self->ssbo_infos[i].usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        self->ssbo_infos[i].size  = sizeof(Uint32);

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

    info.vertex_shader   = self->vertshdr;
    info.fragment_shader = self->fragshdr;

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

bool FG_AmbientLightFilter(Uint32 mask, const void *light)
{
    return ((const FG_DirectLight *)light)->mask & mask &&
           (((const FG_DirectLight *)light)->direction.x != 0.0F ||
           ((const FG_DirectLight *)light)->direction.y != 0.0F ||
           ((const FG_DirectLight *)light)->direction.z != 0.0F);
}

bool FG_OmniLightFilter(Uint32 mask, const void *light)
{
    return ((const FG_OmniLight *)light)->mask & mask &&
           0.0F < ((const FG_OmniLight *)light)->radius;
}

bool FG_ShadingStageSubCopy(FG_ShadingStage *self,
                            SDL_GPUCopyPass *cpypass,
                            Uint8            dst,
                            Uint32          *dst_size,
                            const void      *src,
                            Uint32           src_count,
                            Uint8            size,
                            Uint32           mask,
                            FG_LightFilter   filter)
{
    const Uint8 *it       = src;
    const void  *end      = it + src_count * size;
    Uint32       count    = 0;
    Uint8       *transmem = NULL;
    Uint32       i        = 0;

    if (self->capacity < src_count) {
        self->capacity = src_count;

        self->lights = SDL_realloc(
            self->lights, self->capacity * sizeof(*self->lights));
        if (!self->lights) return false;
    }

    for (it = src, count = 0; it != end; it += size) {
        if (filter(mask, it)) self->lights[count++] = it;
    }

    if (!count) return true;

    *dst_size = count * size;

    if (self->ssbo_infos[dst].size < *dst_size) {
        self->ssbo_infos[dst].size = *dst_size;

        SDL_ReleaseGPUBuffer(self->device, self->ssbos[dst]);
        self->ssbos[dst] = SDL_CreateGPUBuffer(self->device, self->ssbo_infos + dst);
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

    for (i = 0; i != count; ++i, transmem += size) {
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
               &self->ubo.directs_size,
               info->directs,
               info->direct_count,
               sizeof(*info->directs),
               mask,
               FG_AmbientLightFilter
           ) &&
           FG_ShadingStageSubCopy(
               self,
               cpypass,
               1,
               &self->ubo.omnis_size,
               info->omnis,
               info->omni_count,
               sizeof(*info->omnis),
               mask,
               FG_OmniLightFilter
           );
}

void FG_ShadingStageDraw(FG_ShadingStage      *self,
                         SDL_GPUCommandBuffer *cmdbuf,
                         SDL_GPURenderPass    *rndrpass,
                         const FG_Camera      *camera)
{
    self->ubo.origo = camera->transf.transl;
    if (camera->env) {
        self->ubo.ambient = camera->env->light;
        self->ubo.shine   = camera->env->shine;
    }
    else {
        self->ubo.ambient = (FG_Vec3){ .x = 1.0F, .y = 1.0F, .z = 1.0F };
        self->ubo.shine   = 32.0F;
    }

    SDL_BindGPUFragmentSamplers(
        rndrpass, 0, self->sampler_binds, SDL_arraysize(self->sampler_binds));
    SDL_BindGPUFragmentStorageBuffers(rndrpass, 0, self->ssbos, FG_LIGHT_VARIANTS);
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
    SDL_free(self->lights);
    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        SDL_ReleaseGPUSampler(self->device, self->sampler_binds[i].sampler);
    }
    SDL_ReleaseGPUShader(self->device, self->fragshdr);
    SDL_ReleaseGPUShader(self->device, self->vertshdr);
    SDL_free(self);
}
