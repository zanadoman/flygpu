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

#include <stddef.h>

typedef struct {
    FG_Mat4      mvpmat;
    FG_QuadColor color;
    FG_Rect      texcoords;
} FG_Quad3VertIn;

typedef struct
{
    SDL_GPUTexture *albedo;
    Uint32          count;
    Uint32          padding0;
} FG_Quad3Batch;

struct FG_Quad3Stage
{
    SDL_GPUDevice                    *device;
    SDL_GPUTexture                   *nulltex;
    const FG_Quad3                  **instances;
    FG_Quad3Batch                    *batches;
    Uint32                            capacity;
    Uint32                            count;
    SDL_GPUBufferCreateInfo           vertbuf_info;
    Uint32                            padding0;
    SDL_GPUBufferBinding              vertbuf_bind;
    SDL_GPUTransferBufferCreateInfo   transbuf_info;
    Uint32                            padding1;
    SDL_GPUTransferBuffer            *transbuf;
    SDL_GPUShader                    *vertspv;
    SDL_GPUShader                    *fragspv;
    SDL_GPUGraphicsPipeline          *pipeline;
    SDL_GPUTextureSamplerBinding      texsampl_bind;
};

Sint32 FG_CompareQuad3s(const void *lhs, const void *rhs);

FG_Quad3Stage *FG_CreateQuad3Stage(SDL_GPUDevice        *device,
                                   SDL_GPUTextureFormat  colortarg_fmt,
                                   SDL_GPUTexture       *nulltex)
{
    FG_Quad3Stage                     *self        = SDL_calloc(1, sizeof(*self));
    SDL_GPUVertexAttribute             vertattrs[] = {
        { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, mvpmat.m[0 * FG_DIMS_VEC4]) },
        { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, mvpmat.m[1 * FG_DIMS_VEC4]) },
        { .location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, mvpmat.m[2 * FG_DIMS_VEC4]) },
        { .location = 3, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, mvpmat.m[3 * FG_DIMS_VEC4]) },
        { .location = 4, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, color.tl) },
        { .location = 5, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, color.bl) },
        { .location = 6, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, color.br) },
        { .location = 7, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3VertIn, color.tr) },
        { .location = 8, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = offsetof(FG_Quad3VertIn, texcoords.tl) },
        { .location = 9, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = offsetof(FG_Quad3VertIn, texcoords.br) }
    };
    SDL_GPUGraphicsPipelineCreateInfo  info        = {
        .vertex_input_state  = {
            .vertex_buffer_descriptions = &(SDL_GPUVertexBufferDescription){
                .pitch      = sizeof(FG_Quad3VertIn),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE
            },
            .num_vertex_buffers         = 1,
            .vertex_attributes          = vertattrs,
            .num_vertex_attributes      = SDL_arraysize(vertattrs)
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

    self->nulltex = nulltex;

    self->vertbuf_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

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

    info.vertex_shader   = self->vertspv;
    info.fragment_shader = self->fragspv;

    self->pipeline = SDL_CreateGPUGraphicsPipeline(self->device, &info);
    if (!self->pipeline) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    self->texsampl_bind.sampler = SDL_CreateGPUSampler(
        self->device, &(SDL_GPUSamplerCreateInfo){ .props = 0 });
    if (!self->texsampl_bind.sampler) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    return self;
}

Sint32 FG_CompareQuad3s(const void *lhs, const void *rhs)
{
    const FG_Quad3 *a = *(FG_Quad3 *const*)lhs;
    const FG_Quad3 *b = *(FG_Quad3 *const*)rhs;

    if (a->albedo < b->albedo) return -1;
    if (b->albedo < a->albedo) return 1;
    return 0;
}

bool FG_Quad3StageCopy(FG_Quad3Stage               *self,
                       SDL_GPUCopyPass             *cpypass,
                       float                        view_z,
                       const FG_Mat4               *vpmat,
                       const FG_Quad3StageDrawInfo *info)
{
    FG_Quad3VertIn *transmem = NULL;
    Uint32          size     = info->count * sizeof(*transmem);
    Uint32          i        = 0;
    FG_Mat4         modelmat = { .m = { 0.0F } };
    Uint32          j        = 0;

    if (!size) return true;

    if (self->capacity < info->count) {
        self->capacity = info->count;

        self->instances = SDL_realloc(
            self->instances, self->capacity * sizeof(*self->instances));
        if (!self->instances) return false;

        self->batches = SDL_realloc(
            self->batches, self->capacity * sizeof(*self->batches));
        if (!self->batches) return false;
    }

    for (i = 0, self->count = 0; i != self->capacity; ++i) {
        if (info->instances[i].transform.translation.z < view_z) {
            self->instances[self->count++] = info->instances + i;
        }
    }

    SDL_qsort(
        self->instances, self->count, sizeof(*self->instances), FG_CompareQuad3s);

    self->batches->albedo = (*self->instances)->albedo;
    self->batches->count  = 0;

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

    for (i = 0; i != self->count; ++i, ++transmem) {
        FG_SetModelMat4(&self->instances[i]->transform, &modelmat);
        FG_MulMat4s(vpmat, &modelmat, &transmem->mvpmat);
        transmem->color = self->instances[i]->color;
        transmem->texcoords = self->instances[i]->texcoords;
        if (self->instances[i]->albedo != self->batches[j].albedo) {
            ++j;
            self->batches[j].albedo = self->instances[i]->albedo;
            self->batches[j].count  = 1;
        }
        else ++self->batches[j].count;
    }

    SDL_UnmapGPUTransferBuffer(self->device, self->transbuf);

    SDL_UploadToGPUBuffer(
        cpypass,
        &(SDL_GPUTransferBufferLocation){ .transfer_buffer = self->transbuf },
        &(SDL_GPUBufferRegion){
            .buffer = self->vertbuf_bind.buffer,
            .size   = self->vertbuf_info.size
        },
        false
    );

    return true;
}

void FG_Quad3StageDraw(FG_Quad3Stage *self, SDL_GPURenderPass *rndrpass)
{
    Uint32 i = 0;
    Uint32 j = 0;

    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    SDL_BindGPUVertexBuffers(rndrpass, 0, &self->vertbuf_bind, 1);
    for (i = 0, j = 0; i != self->count; i += self->batches[j].count, ++j) {
        self->texsampl_bind.texture = self->batches[j].albedo;
        if (!self->texsampl_bind.texture) self->texsampl_bind.texture = self->nulltex;
        SDL_BindGPUFragmentSamplers(rndrpass, 0, &self->texsampl_bind, 1);
        SDL_DrawGPUPrimitives(rndrpass, 6, self->batches[j].count, 0, i);
    }
}

void FG_DestroyQuad3Stage(FG_Quad3Stage *self)
{
    if (!self) return;
    SDL_ReleaseGPUSampler(self->device, self->texsampl_bind.sampler);
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    SDL_ReleaseGPUShader(self->device, self->fragspv);
    SDL_ReleaseGPUShader(self->device, self->vertspv);
    SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
    SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
    SDL_free(self->batches);
    SDL_free(self->instances);
    SDL_free(self);
}
