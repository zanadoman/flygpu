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

#include "quad3pline.h"

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"
#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

struct FG_Quad3Pline
{
    SDL_GPUDevice                   *device;
    SDL_GPUGraphicsPipeline         *pipeline;
    SDL_GPUBufferCreateInfo          vertbuf_info;
    Uint32                           padding0;
    SDL_GPUBufferBinding             vertbuf_bind;
    SDL_GPUTransferBufferCreateInfo  transbuf_info;
    Uint32                           padding1;
    SDL_GPUTransferBuffer           *transbuf;
    Uint32                           instances;
    Uint32                           padding2;
};

FG_Quad3Pline *FG_CreateQuad3Pline(SDL_GPUDevice *device, SDL_Window *window)
{
    FG_Quad3Pline                     *self                     = SDL_calloc(1, sizeof(*self));
    SDL_GPUVertexAttribute             vert_attrs[FG_DIMS_VEC4] = {
        { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 0 * sizeof(FG_Vec4) },
        { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 1 * sizeof(FG_Vec4) },
        { .location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 2 * sizeof(FG_Vec4) },
        { .location = 3, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 3 * sizeof(FG_Vec4) }
    };
    SDL_GPUGraphicsPipelineCreateInfo  info                     = {
        .vertex_input_state = {
            .vertex_buffer_descriptions = &(SDL_GPUVertexBufferDescription){
                .pitch      = sizeof(FG_Mat4),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE
            },
            .num_vertex_buffers    = 1,
            .vertex_attributes     = vert_attrs,
            .num_vertex_attributes = sizeof(vert_attrs) / sizeof(*vert_attrs)
        },
        .target_info = {
            .color_target_descriptions = &(SDL_GPUColorTargetDescription){
                .format = SDL_GetGPUSwapchainTextureFormat(device, window)
            },
            .num_color_targets = 1
        }
    };

    if (!self) return NULL;

    self->device = device;

    info.vertex_shader = FG_LoadShader(
        self->device, "./shaders/quad3.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    if (!info.vertex_shader) {
        FG_ReleaseQuad3Pline(self);
        return NULL;
    }

    info.fragment_shader = FG_LoadShader(
        self->device, "./shaders/quad3.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!info.fragment_shader) {
        FG_ReleaseQuad3Pline(self);
        return NULL;
    }

    self->vertbuf_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    self->vertbuf_info.size  = 10000 * sizeof(FG_Mat4);

    self->vertbuf_bind.buffer = SDL_CreateGPUBuffer(self->device, &self->vertbuf_info);
    if (!self->vertbuf_bind.buffer) {
        FG_ReleaseQuad3Pline(self);
        return NULL;
    }

    self->transbuf_info.size = self->vertbuf_info.size;

    self->transbuf = SDL_CreateGPUTransferBuffer(self->device, &self->transbuf_info);
    if (!self->transbuf) {
        FG_ReleaseQuad3Pline(self);
        return NULL;
    }

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    SDL_ReleaseGPUShader(self->device, info.fragment_shader);
    SDL_ReleaseGPUShader(self->device, info.vertex_shader);
    if (!self->pipeline) {
        FG_ReleaseQuad3Pline(self);
        return NULL;
    }

    return self;
}

bool FG_Quad3PlineCopy(FG_Quad3Pline   *self,
                       SDL_GPUCopyPass *cpypass,
                       const FG_Mat4   *projmat,
                       const FG_Quad3  *begin,
                       const FG_Quad3  *end)
{
    FG_Mat4 *vertbuf  = SDL_MapGPUTransferBuffer(self->device, self->transbuf, false);
    FG_Mat4  transmat;

    if (!vertbuf) return false;

    self->instances = (Uint32)(end - begin);
    for (; begin != end; ++begin, ++vertbuf) {
        FG_SetTransMat4(&begin->transform, &transmat);
        FG_MulMat4s(projmat, &transmat, vertbuf);
    }

    SDL_UnmapGPUTransferBuffer(self->device, self->transbuf);
    SDL_UploadToGPUBuffer(
        cpypass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = self->transbuf
        },
        &(SDL_GPUBufferRegion){
            .buffer = self->vertbuf_bind.buffer,
            .size   = self->vertbuf_info.size
        },
        false
    );

    return true;
}

void FG_Quad3PlineDraw(FG_Quad3Pline *self, SDL_GPURenderPass *rndrpass)
{
    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_BindGPUVertexBuffers(rndrpass, 0, &self->vertbuf_bind, 1);
    SDL_DrawGPUPrimitives(rndrpass, 6, self->instances, 0, 0);
}

void FG_ReleaseQuad3Pline(FG_Quad3Pline *self)
{
    if (!self) return;
    SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
    SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    SDL_free(self);
}
