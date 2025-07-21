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
    SDL_GPUGraphicsPipeline *pline;
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

    quad3pline->pline = SDL_CreateGPUGraphicsPipeline(device, &info);
    SDL_ReleaseGPUShader(device, info.fragment_shader);
    SDL_ReleaseGPUShader(device, info.vertex_shader);
    if (!quad3pline->pline) {
        SDL_free(quad3pline);
        return NULL;
    }

    return quad3pline;
}

void FG_Quad3PlineDraw(FG_Quad3Pline        *quad3pline,
                       SDL_GPUCommandBuffer *cmdbuf,
                       SDL_GPURenderPass    *rpass,
                       const FG_Mat4        *projmat,
                       const FG_Quad3       *begin,
                       const FG_Quad3       *end)
{
    SDL_BindGPUGraphicsPipeline(rpass, quad3pline->pline);
    for (const FG_Quad3 *quad3 = begin; quad3 != end; ++quad3) {
        FG_Mat4 transmat;
        FG_SetTransMat4(&quad3->transform, &transmat);
        FG_Mat4 mvpmat;
        FG_MulMat4s(projmat, &transmat, &mvpmat);
        SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvpmat, sizeof(mvpmat));
        SDL_DrawGPUPrimitives(rpass, 6, 1, 0, 0);
    }
}

void FG_ReleaseQuad3Pline(SDL_GPUDevice *device, FG_Quad3Pline *quad3pline)
{
    SDL_ReleaseGPUGraphicsPipeline(device, quad3pline->pline);
    SDL_free(quad3pline);
}
