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

#include "environment_stage.h"

#include "../include/flygpu/flygpu.h"
#include "linalg.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stddef.h>

struct FG_EnvironmentStage
{
    SDL_GPUDevice                *device;
    SDL_GPUShader                *vertshdr;
    SDL_GPUShader                *fragshdr;
    SDL_GPUTextureSamplerBinding  sampler_bind;
    SDL_GPUGraphicsPipeline      *pipeline;
};

typedef struct
{
    FG_Mat4 matrix;
    FG_Vec3 color_tl;
    Uint32  padding0;
    FG_Vec3 color_bl;
    Uint32  padding1;
    FG_Vec3 color_br;
    Uint32  padding2;
    FG_Vec3 color_tr;
    Uint32  padding3;
    FG_AABB coords;
} FG_EnvironmentStageUBO;

FG_EnvironmentStage * FG_CreateEnvironmentStage(SDL_GPUDevice        *device,
                                                SDL_GPUTextureFormat  targbuf_fmt)
{
    FG_EnvironmentStage               *self         = SDL_calloc(1, sizeof(*self));
    SDL_GPUSamplerCreateInfo           sampler_info = {
        .min_filter = SDL_GPU_FILTER_LINEAR
    };
    SDL_GPUGraphicsPipelineCreateInfo  info         = {
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
        device, "environment.vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 1);
    if (!self->vertshdr) {
        FG_DestroyEnvironmentStage(self);
        return NULL;
    }

    self->fragshdr = FG_LoadShader(
        device, "environment.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0);
    if (!self->fragshdr) {
        FG_DestroyEnvironmentStage(self);
        return NULL;
    }

    self->sampler_bind.sampler = SDL_CreateGPUSampler(self->device, &sampler_info);
    if (!self->sampler_bind.sampler) {
        FG_DestroyEnvironmentStage(self);
        return NULL;
    }

    info.vertex_shader   = self->vertshdr;
    info.fragment_shader = self->fragshdr;

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    if (!self->pipeline) {
        FG_DestroyEnvironmentStage(self);
        return NULL;
    }

    return self;
}

void FG_EnvironmentStageDraw(FG_EnvironmentStage  *self,
                             SDL_GPUCommandBuffer *cmdbuf,
                             SDL_GPURenderPass    *rndrpass,
                             float                 width,
                             float                 height,
                             const FG_Camera      *camera,
                             SDL_GPUTexture       *fallback)
{
    FG_Vec2                scale = { .x = FG_SQRT2F, .y = FG_SQRT2F };
    FG_EnvironmentStageUBO ubo   = { 0 };

    if (width < height) {
        scale.y = FG_hypot1f(width / height);
        scale.x = scale.y * height / width;
        FG_SetEnvMat4(&scale, camera->transf.rotation, &ubo.matrix);
    }
    else if (height < width) {
        scale.x = FG_hypot1f(height / width);
        scale.y = scale.x * width / height;
        FG_SetEnvMat4(&scale, camera->transf.rotation, &ubo.matrix);
    }
    else FG_SetEnvMat4(&scale, camera->transf.rotation, &ubo.matrix);
    if (camera->env) {
        ubo.color_tl = camera->env->color.tl;
        ubo.color_bl = camera->env->color.bl;
        ubo.color_br = camera->env->color.br;
        ubo.color_tr = camera->env->color.tr;
        ubo.coords   = camera->env->coords;

        self->sampler_bind.texture = camera->env->texture ? camera->env->texture
                                                          : fallback;
    }
    else self->sampler_bind.texture = fallback;

    SDL_PushGPUVertexUniformData(cmdbuf, 0, &ubo, sizeof(ubo));
    SDL_BindGPUFragmentSamplers(rndrpass, 0, &self->sampler_bind, 1);
    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_DrawGPUPrimitives(rndrpass, 6, 1, 0, 0);
}

void FG_DestroyEnvironmentStage(FG_EnvironmentStage *self)
{
    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    SDL_ReleaseGPUSampler(self->device, self->sampler_bind.sampler);
    SDL_ReleaseGPUShader(self->device, self->fragshdr);
    SDL_ReleaseGPUShader(self->device, self->vertshdr);
    SDL_free(self);
}
