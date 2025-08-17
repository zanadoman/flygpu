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

#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    const FG_Material *material;
    Uint32             count;
    Uint8              padding0[4];
} FG_Quad3Batch;

typedef struct
{
    FG_Mat4      modelmat;
    FG_Mat4      mvpmat;
    FG_Mat3      tbnmat;
    FG_QuadColor color;
    FG_Rect      coords;
} FG_Quad3In;

struct FG_Quad3Stage
{
    SDL_GPUDevice                 *device;
    const FG_Material             *material;
    SDL_GPUShader                 *vertspv;
    SDL_GPUShader                 *fragspv;
    Uint32                         capacity;
    Uint32                         count;
    const FG_Quad3               **instances;
    FG_Quad3Batch                 *batches;
    SDL_GPUBufferCreateInfo        vertbuf_info;
    Uint8                          padding0[4];
    SDL_GPUBufferBinding           vertbuf_bind;
    SDL_GPUTransferBuffer         *transbuf;
    SDL_GPUTextureSamplerBinding   sampler_binds[3];
    SDL_GPUGraphicsPipeline       *pipeline;
};

static Sint32 FG_CompareQuad3s(const void *lhs, const void *rhs);

FG_Quad3Stage *FG_CreateQuad3Stage(SDL_GPUDevice *device, const FG_Material *material)
{
    Uint8                              i                            = 0;
    SDL_GPUColorTargetDescription      targbuf_descs[FG_GBUF_COUNT];
    FG_Quad3Stage                     *self                         = SDL_calloc(1, sizeof(*self));
    SDL_GPUVertexAttribute             vertattrs[]                  = {
        { .location = 0,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, modelmat.m[0 * FG_DIMS_VEC4]) },
        { .location = 1,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, modelmat.m[1 * FG_DIMS_VEC4]) },
        { .location = 2,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, modelmat.m[2 * FG_DIMS_VEC4]) },
        { .location = 3,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, modelmat.m[3 * FG_DIMS_VEC4]) },
        { .location = 4,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, mvpmat.m[0 * FG_DIMS_VEC4]) },
        { .location = 5,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, mvpmat.m[1 * FG_DIMS_VEC4]) },
        { .location = 6,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, mvpmat.m[2 * FG_DIMS_VEC4]) },
        { .location = 7,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, mvpmat.m[3 * FG_DIMS_VEC4]) },
        { .location = 8,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, tbnmat.m[0 * FG_DIMS_VEC3]) },
        { .location = 9,  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, tbnmat.m[1 * FG_DIMS_VEC3]) },
        { .location = 10, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, tbnmat.m[2 * FG_DIMS_VEC3]) },
        { .location = 11, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, color.tl) },
        { .location = 12, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, color.bl) },
        { .location = 13, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, color.br) },
        { .location = 14, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = offsetof(FG_Quad3In, color.tr) },
        { .location = 15, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = offsetof(FG_Quad3In, coords.tl) }
    };
    SDL_GPUGraphicsPipelineCreateInfo  info                         = {
        .vertex_input_state  = {
            .vertex_buffer_descriptions = &(SDL_GPUVertexBufferDescription){
                .pitch      = sizeof(FG_Quad3In),
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
            .color_target_descriptions = targbuf_descs,
            .num_color_targets         = SDL_arraysize(targbuf_descs),
            .depth_stencil_format      = FG_DEPTH_FORMAT,
            .has_depth_stencil_target  = true
        }
    };

    for (i = 0; i != SDL_arraysize(targbuf_descs); ++i) {
        targbuf_descs[i] = (SDL_GPUColorTargetDescription){ .format = FG_GBUF_FORMAT };
    }

    if (!self) return NULL;

    self->device = device;

    self->material = material;

    self->vertspv = FG_LoadShader(
        self->device, "./shaders/quad3.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0);
    if (!self->vertspv) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    self->fragspv = FG_LoadShader(
        self->device,
        "./shaders/quad3.frag.spv",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        SDL_arraysize(self->sampler_binds),
        0,
        0
    );
    if (!self->fragspv) {
        FG_DestroyQuad3Stage(self);
        return NULL;
    }

    self->vertbuf_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        self->sampler_binds[i].sampler = SDL_CreateGPUSampler(
            self->device,
            &(SDL_GPUSamplerCreateInfo){
                .min_filter  = SDL_GPU_FILTER_LINEAR,
                .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                .max_lod     = 1000.0F
            }
        );
        if (!self->sampler_binds[i].sampler) {
            FG_DestroyQuad3Stage(self);
            return NULL;
        }
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

Sint32 FG_CompareQuad3s(const void *lhs, const void *rhs)
{
    const FG_Quad3 *a = *(FG_Quad3 *const *)lhs;
    const FG_Quad3 *b = *(FG_Quad3 *const *)rhs;

    if (a->material < b->material) return -1;
    if (b->material < a->material) return 1;
    return 0;
}

bool FG_Quad3StageCopy(FG_Quad3Stage               *self,
                       SDL_GPUCopyPass             *cpypass,
                       float                        near,
                       float                        far,
                       const FG_Mat4               *vpmat,
                       const FG_Quad3StageDrawInfo *info)
{
    Uint32      i        = 0;
    Uint32      size     = 0;
    FG_Quad3In *transmem = NULL;
    Uint32      j        = 0;

    if (self->capacity < info->count) {
        self->capacity = info->count;

        self->instances = SDL_realloc(
            self->instances, self->capacity * sizeof(*self->instances));
        if (!self->instances) return false;

        self->batches = SDL_realloc(
            self->batches, self->capacity * sizeof(*self->batches));
        if (!self->batches) return false;
    }

    for (i = 0, self->count = 0; i != info->count; ++i) {
        if (far < info->instances[i].transform.translation.z &&
            info->instances[i].transform.translation.z < near
        ) {
            self->instances[self->count++] = info->instances + i;
        }
    }

    if (!self->count) return true;

    SDL_qsort(
        self->instances, self->count, sizeof(*self->instances), FG_CompareQuad3s);

    self->batches->material = (*self->instances)->material;
    self->batches->count    = 0;

    size = self->count * sizeof(*transmem);

    if (self->vertbuf_info.size < size) {
        SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
        self->vertbuf_info.size   = size;
        self->vertbuf_bind.buffer = SDL_CreateGPUBuffer(
            self->device, &self->vertbuf_info);
        if (!self->vertbuf_bind.buffer) return false;

        SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
        self->transbuf = SDL_CreateGPUTransferBuffer(
            self->device,
            &(SDL_GPUTransferBufferCreateInfo){ .size = self->vertbuf_info.size }
        );
        if (!self->transbuf) return false;
    }

    transmem = SDL_MapGPUTransferBuffer(self->device, self->transbuf, true);
    if (!transmem) return false;

    for (i = 0, j = 0; i != self->count; ++i, ++transmem) {
        FG_SetModelMat4(&self->instances[i]->transform, &transmem->modelmat);
        FG_MulMat4s(vpmat, &transmem->modelmat, &transmem->mvpmat);
        FG_SetTBNMat3(self->instances[i]->transform.rotation, &transmem->tbnmat);
        transmem->color  = self->instances[i]->color;
        transmem->coords = self->instances[i]->coords;
        if (self->instances[i]->material != self->batches[j].material) {
            ++j;
            self->batches[j].material = self->instances[i]->material;
            self->batches[j].count    = 1;
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

    if (!self->count) return;

    SDL_BindGPUVertexBuffers(rndrpass, 0, &self->vertbuf_bind, 1);

    SDL_BindGPUGraphicsPipeline(rndrpass, self->pipeline);
    for (i = 0, j = 0; i != self->count; i += self->batches[j++].count) {
        if (self->batches[j].material) {
            self->sampler_binds[0].texture = self->batches[j].material->albedo;
            if (!self->sampler_binds[0].texture) {
                self->sampler_binds[0].texture = self->material->albedo;
            }
            self->sampler_binds[1].texture = self->batches[j].material->specular;
            if (!self->sampler_binds[1].texture) {
                self->sampler_binds[1].texture = self->material->specular;
            }
            self->sampler_binds[2].texture = self->batches[j].material->normal;
            if (!self->sampler_binds[2].texture) {
                self->sampler_binds[2].texture = self->material->normal;
            }
        }
        else {
            self->sampler_binds[0].texture = self->material->albedo;
            self->sampler_binds[1].texture = self->material->specular;
            self->sampler_binds[2].texture = self->material->normal;
        }
        SDL_BindGPUFragmentSamplers(
            rndrpass, 0, self->sampler_binds, SDL_arraysize(self->sampler_binds));
        SDL_DrawGPUPrimitives(rndrpass, 6, self->batches[j].count, 0, i);
    }
}

void FG_DestroyQuad3Stage(FG_Quad3Stage *self)
{
    Uint8 i = 0;

    if (!self) return;
    SDL_ReleaseGPUGraphicsPipeline(self->device, self->pipeline);
    for (i = 0; i != SDL_arraysize(self->sampler_binds); ++i) {
        SDL_ReleaseGPUSampler(self->device, self->sampler_binds[i].sampler);
    }
    SDL_ReleaseGPUTransferBuffer(self->device, self->transbuf);
    SDL_ReleaseGPUBuffer(self->device, self->vertbuf_bind.buffer);
    SDL_free(self->batches);
    SDL_free(self->instances);
    SDL_ReleaseGPUShader(self->device, self->fragspv);
    SDL_ReleaseGPUShader(self->device, self->vertspv);
    SDL_free(self);
}
