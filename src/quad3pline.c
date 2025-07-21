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

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"
#include "shader.h"

struct FG_Quad3Pline
{
    SDL_GPUGraphicsPipeline        *pline;
    SDL_GPUBufferCreateInfo         vertbuf_info;
    SDL_GPUBufferBinding            vertbuf_bind;
    SDL_GPUTransferBufferCreateInfo transbuf_info;
    SDL_GPUTransferBuffer          *transbuf;
};

FG_Quad3Pline *FG_CreateQuad3Pline(SDL_GPUDevice *device, SDL_Window *window)
{
    FG_Quad3Pline *quad3pline = SDL_malloc(sizeof(*quad3pline));
    if (!quad3pline) return NULL;

    SDL_GPUGraphicsPipelineCreateInfo info = {
        .target_info = {
            .color_target_descriptions =
                &(SDL_GPUColorTargetDescription){
                    .format = SDL_GetGPUSwapchainTextureFormat(device, window)},
            .num_color_targets = 1}};

    info.vertex_shader = FG_LoadShader(device, "./shaders/quad.vert.spv",
                                       SDL_GPU_SHADERSTAGE_VERTEX, 1);
    if (!info.vertex_shader) {
        SDL_free(quad3pline);
        return NULL;
    }

    info.fragment_shader = FG_LoadShader(device, "./shaders/quad.frag.spv",
                                         SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
    if (!info.fragment_shader) {
        SDL_ReleaseGPUShader(device, info.vertex_shader);
        SDL_free(quad3pline);
        return NULL;
    }

    quad3pline->vertbuf_info = (SDL_GPUBufferCreateInfo){
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(FG_Mat4) * 10000};
    quad3pline->vertbuf_bind = (SDL_GPUBufferBinding){
        .buffer = SDL_CreateGPUBuffer(device, &quad3pline->vertbuf_info)};
    if (!quad3pline->vertbuf_bind.buffer) {
        SDL_ReleaseGPUShader(device, info.fragment_shader);
        SDL_ReleaseGPUShader(device, info.vertex_shader);
        SDL_free(quad3pline);
        return NULL;
    }
    info.vertex_input_state.vertex_buffer_descriptions =
        &(SDL_GPUVertexBufferDescription){
            .pitch      = sizeof(FG_Mat4),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE};
    info.vertex_input_state.num_vertex_buffers = 1;
    SDL_GPUVertexAttribute attrs[] = {
        // clang-format off
        {.location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(FG_Vec4) * 0},
        {.location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(FG_Vec4) * 1},
        {.location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(FG_Vec4) * 2},
            {.location = 3, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(FG_Vec4) * 3},
    };
    info.vertex_input_state.vertex_attributes = attrs;
    info.vertex_input_state.num_vertex_attributes = 4;

    quad3pline->transbuf_info = (SDL_GPUTransferBufferCreateInfo){
        .size = quad3pline->vertbuf_info.size};
    quad3pline->transbuf =
        SDL_CreateGPUTransferBuffer(device, &quad3pline->transbuf_info);
    if (!quad3pline->transbuf) {
        SDL_ReleaseGPUBuffer(device, quad3pline->vertbuf_bind.buffer);
        SDL_ReleaseGPUShader(device, info.fragment_shader);
        SDL_ReleaseGPUShader(device, info.vertex_shader);
        SDL_free(quad3pline);
        return NULL;
    }

    quad3pline->pline = SDL_CreateGPUGraphicsPipeline(device, &info);
    SDL_ReleaseGPUShader(device, info.fragment_shader);
    SDL_ReleaseGPUShader(device, info.vertex_shader);
    if (!quad3pline->pline) {
        SDL_free(quad3pline);
        return NULL;
    }

    return quad3pline;
}

bool FG_Quad3PlineCopy(FG_Quad3Pline   *quad3pline,
                       SDL_GPUDevice   *device,
                       SDL_GPUCopyPass *cpass,
                       const FG_Mat4   *projmat,
                       const FG_Quad3  *begin,
                       const FG_Quad3  *end)
{
    FG_Mat4 *vertbuf =
        SDL_MapGPUTransferBuffer(device, quad3pline->transbuf, false);
    if (!vertbuf) return false;

    for (const FG_Quad3 *quad3 = begin; quad3 != end; ++quad3, ++vertbuf) {
        FG_Mat4 transmat;
        FG_SetTransMat4(&quad3->transform, &transmat);
        FG_MulMat4s(projmat, &transmat, vertbuf);
    }

    SDL_UnmapGPUTransferBuffer(device, quad3pline->transbuf);
    SDL_UploadToGPUBuffer(
        cpass,
        &(SDL_GPUTransferBufferLocation){.transfer_buffer =
                                             quad3pline->transbuf},
        &(SDL_GPUBufferRegion){.buffer = quad3pline->vertbuf_bind.buffer,
                               .size   = quad3pline->vertbuf_info.size},
        false);

    return true;
}

void FG_Quad3PlineDraw(FG_Quad3Pline     *quad3pline,
                       SDL_GPURenderPass *rpass,
                       Uint32             instances)
{
    SDL_BindGPUGraphicsPipeline(rpass, quad3pline->pline);
    SDL_BindGPUVertexBuffers(rpass, 0, &quad3pline->vertbuf_bind, 1);
    SDL_DrawGPUPrimitives(rpass, 6, instances, 0, 0);
}

void FG_ReleaseQuad3Pline(SDL_GPUDevice *device, FG_Quad3Pline *quad3pline)
{
    SDL_ReleaseGPUTransferBuffer(device, quad3pline->transbuf);
    SDL_ReleaseGPUBuffer(device, quad3pline->vertbuf_bind.buffer);
    SDL_ReleaseGPUGraphicsPipeline(device, quad3pline->pline);
    SDL_free(quad3pline);
}
