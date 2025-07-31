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

#include "quad3stage.h"

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#define VERTBUF_MAT4S 2
#define VERTBUF_PITCH (VERTBUF_MAT4S * sizeof(FG_Mat4))
#define VERTBUF_ATTRS (VERTBUF_PITCH / sizeof(FG_Vec4))

struct FG_Quad3Stage
{
    SDL_GPUDevice                   *device;
    SDL_GPUShader                   *vertspv;
    SDL_GPUShader                   *fragspv;
    SDL_GPUBufferCreateInfo          vertbuf_info;
    Uint32                           padding0;
    SDL_GPUBufferBinding             vertbuf_bind;
    SDL_GPUTransferBufferCreateInfo  transbuf_info;
    Uint32                           padding1;
    SDL_GPUTransferBuffer           *transbuf;
    SDL_GPUTextureSamplerBinding     texsampl_bind;
    SDL_GPUGraphicsPipeline         *pipeline;
};

FG_Quad3Stage *FG_CreateQuad3Stage(SDL_GPUDevice        *device,
                                   SDL_GPUTextureFormat  colortarg_fmt)
{
    FG_Quad3Stage                     *self = SDL_calloc(1, sizeof(*self));
    SDL_GPUGraphicsPipelineCreateInfo  info = {
        .vertex_input_state  = {
            .vertex_buffer_descriptions = &(SDL_GPUVertexBufferDescription){
                .pitch      = VERTBUF_PITCH,
                .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE
            },
            .num_vertex_buffers         = 1,
            .vertex_attributes          = (SDL_GPUVertexAttribute[VERTBUF_ATTRS]){
                { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 0 * sizeof(FG_Vec4) },
                { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 1 * sizeof(FG_Vec4) },
                { .location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 2 * sizeof(FG_Vec4) },
                { .location = 3, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 3 * sizeof(FG_Vec4) },
                { .location = 4, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 4 * sizeof(FG_Vec4) },
                { .location = 5, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 5 * sizeof(FG_Vec4) },
                { .location = 6, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 6 * sizeof(FG_Vec4) },
                { .location = 7, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 7 * sizeof(FG_Vec4) }
            },
            .num_vertex_attributes      = VERTBUF_ATTRS
        },
        .depth_stencil_state = {
            .compare_op         = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test  = true,
            .enable_depth_write = true
        },
        .target_info         = {
            .color_target_descriptions = &(SDL_GPUColorTargetDescription){
                .format = colortarg_fmt
            },
            .num_color_targets         = 1,
            .depth_stencil_format      = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
            .has_depth_stencil_target  = true
        }
    };

    if (!self) return NULL;

    self->device = device;

    self->vertspv = FG_LoadShader(
        self->device, "./shaders/quad3.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0);
    if (!self->vertspv) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    self->fragspv = FG_LoadShader(
        self->device, "./shaders/quad3.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1);
    if (!self->fragspv) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    self->vertbuf_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

    self->texsampl_bind.sampler = SDL_CreateGPUSampler(
        self->device, &(SDL_GPUSamplerCreateInfo){ .props = 0 });
    if (!self->texsampl_bind.sampler) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    info.vertex_shader   = self->vertspv;
    info.fragment_shader = self->fragspv;

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    if (!self->pipeline) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    return self;
}

bool FG_Quad3StageCopy(FG_Quad3Stage               *self,
                       SDL_GPUCopyPass             *cpypass,
                       const FG_Mat4               *vpmat,
                       const FG_Quad3StageDrawInfo *info)
{
    Uint32   size     = info->count * VERTBUF_PITCH;
    FG_Mat4 *transmem = NULL;
    Uint32   i        = 0;
    FG_Mat4  modelmat = { .data = { 0.0F } };

    if (!size) return true;

    if (self->vertbuf_info.size < size) {
        SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
        self->vertbuf_info.size   = size;
        self->vertbuf_bind.buffer = SDL_CreateGPUBuffer(
            self->device, &self->vertbuf_info);
        if (!self->vertbuf_bind.buffer) return false;

        SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
        self->transbuf_info.size = size;
        self->transbuf           = SDL_CreateGPUTransferBuffer(
            self->device, &self->transbuf_info);
        if (!self->transbuf) return false;
    }

    transmem = SDL_MapGPUTransferBuffer(self->device, self->transbuf, true);
    if (!transmem) return false;

    for (i = 0; i != info->count; ++i, transmem += VERTBUF_MAT4S) {
        FG_SetTransMat4(&info->instances[i].transform, &modelmat);
        FG_MulMat4s(vpmat, &modelmat, transmem);
        transmem[1].cols[0] = info->instances[i].color.bl;
        transmem[1].cols[1] = info->instances[i].color.br;
        transmem[1].cols[2] = info->instances[i].color.tr;
        transmem[1].cols[3] = info->instances[i].color.tl;
    }

    SDL_UnmapGPUTransferBuffer(self->device, self->transbuf);

    SDL_UploadToGPUBuffer(
        cpypass,
        &(SDL_GPUTransferBufferLocation){ .transfer_buffer = self->transbuf },
        &(SDL_GPUBufferRegion){
            .buffer = self->vertbuf_bind.buffer,
            .size   = self->vertbuf_info.size
        },
        true
    );

    return true;
}

void FG_Quad3StageDraw(FG_Quad3Stage               *self,
                       SDL_GPURenderPass           *rndrpass,
                       const FG_Quad3StageDrawInfo *info)
{
    Uint32 i = 0;

    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_BindGPUVertexBuffers(rndrpass, 0, &self->vertbuf_bind, 1);
    for (i = 0; i != info->count; ++i) {
        self->texsampl_bind.texture = info->instances[i].texture;
        SDL_BindGPUFragmentSamplers(rndrpass, 0, &self->texsampl_bind, 1);
        SDL_DrawGPUPrimitives(rndrpass, 6, 1, 0, i);
    }
}

void FG_DestroyQuad3Stage(FG_Quad3Stage *self)
{
    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    SDL_ReleaseGPUSampler(self->device, self->texsampl_bind.sampler);
    SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
    SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
    SDL_ReleaseGPUShader(self->device, self->fragspv);
    SDL_ReleaseGPUShader(self->device, self->vertspv);
    SDL_free(self);
}
