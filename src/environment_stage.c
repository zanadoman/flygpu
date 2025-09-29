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

#include "environment_stage.h"

#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stddef.h>

struct FG_EnvironmentStage
{
    SDL_GPUDevice           *device;
    SDL_GPUShader           *vertshdr;
    SDL_GPUShader           *fragshdr;
    SDL_GPUGraphicsPipeline *pipeline;
};

FG_EnvironmentStage * FG_CreateEnvironmentStage(SDL_GPUDevice        *device,
                                                SDL_GPUTextureFormat  targbuf_fmt)
{
    FG_EnvironmentStage               *self = SDL_calloc(1, sizeof(*self));
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
        device, "environment.vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0);
    if (!self->vertshdr) {
        FG_DestroyEnvironmentStage(self);
        return NULL;
    }

    self->fragshdr = FG_LoadShader(
        device, "environment.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0);
    if (!self->fragshdr) {
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

void FG_EnvironmentStageDraw(FG_EnvironmentStage *self, SDL_GPURenderPass *rndrpass)
{
    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_DrawGPUPrimitives(rndrpass, 6, 1, 0, 0);
}

void FG_DestroyEnvironmentStage(FG_EnvironmentStage *self)
{
    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    SDL_ReleaseGPUShader(self->device, self->fragshdr);
    SDL_ReleaseGPUShader(self->device, self->vertshdr);
    SDL_free(self);
}
