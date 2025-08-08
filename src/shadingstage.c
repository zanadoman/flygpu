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

#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stddef.h>

struct FG_ShadingStage
{
    SDL_GPUDevice                *device;
    SDL_GPUShader                *vertspv;
    SDL_GPUShader                *fragspv;
    SDL_GPUTextureSamplerBinding  sampler_binds[FG_GBUF_LOCATION_COUNT];
    SDL_GPUGraphicsPipeline      *pipeline;
};

FG_ShadingStage *FG_CreateShadingStage(SDL_GPUDevice        *device,
                                       SDL_GPUTextureFormat  swapctarg_format)
{
    FG_ShadingStage                   *self = SDL_calloc(1, sizeof(*self));
    Uint8                              i    = 0;
    SDL_GPUGraphicsPipelineCreateInfo  info = {
        .target_info = {
            .color_target_descriptions = &(SDL_GPUColorTargetDescription){
                .format = swapctarg_format
            },
            .num_color_targets         = 1
        }
    };

    if (!self) return NULL;

    self->device = device;

    self->vertspv = FG_LoadShader(
        self->device, "./shaders/shading.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0);
    if (!self->vertspv) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    self->fragspv = FG_LoadShader(
        self->device,
        "./shaders/shading.frag.spv",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        SDL_arraysize(self->sampler_binds)
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

    info.vertex_shader   = self->vertspv;
    info.fragment_shader = self->fragspv;

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    if (!self->pipeline) {
        FG_DestroyShadingStage(self);
        return NULL;
    }

    return self;
}

void FG_ShadingStageCopy(FG_ShadingStage        *self,
                         SDL_GPUColorTargetInfo *gbuftarg_infos)
{
    Uint8 i = 0;

    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        self->sampler_binds[i].texture = gbuftarg_infos[i].texture;
    }
}

void FG_ShadingStageDraw(FG_ShadingStage *self, SDL_GPURenderPass *rndrpass)
{
    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_BindGPUFragmentSamplers(
        rndrpass, 0, self->sampler_binds, SDL_arraysize(self->sampler_binds));
    SDL_DrawGPUPrimitives(rndrpass, 6, 1, 0, 0);
}

void FG_DestroyShadingStage(FG_ShadingStage *self)
{
    Uint8 i = 0;

    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        SDL_ReleaseGPUSampler(self->device, self->sampler_binds[i].sampler);
    }
    SDL_ReleaseGPUShader(self->device, self->fragspv);
    SDL_ReleaseGPUShader(self->device, self->vertspv);
    SDL_free(self);
}
