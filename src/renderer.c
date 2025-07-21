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

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"
#include "quad3pline.h"

struct FG_Renderer
{
    SDL_Window            *window;
    SDL_GPUDevice         *device;
    SDL_GPUColorTargetInfo target;
    FG_Quad3Pline         *quad3pline;
};

FG_Renderer *FG_CreateRenderer(SDL_Window *window, bool vsync)
{
    FG_Renderer *renderer = SDL_malloc(sizeof(*renderer));
    if (!renderer) return NULL;

    renderer->window = window;

    renderer->device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
    if (!renderer->device) {
        SDL_free(renderer);
        return NULL;
    }

    if (!SDL_ClaimWindowForGPUDevice(renderer->device, renderer->window)) {
        SDL_DestroyGPUDevice(renderer->device);
        SDL_free(renderer);
        return NULL;
    }

    if (!SDL_SetGPUSwapchainParameters(renderer->device, renderer->window,
                                       SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                       vsync ? SDL_GPU_PRESENTMODE_VSYNC
                                             : SDL_GPU_PRESENTMODE_IMMEDIATE)) {
        SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
        SDL_DestroyGPUDevice(renderer->device);
        SDL_free(renderer);
        return NULL;
    }

    renderer->target =
        (SDL_GPUColorTargetInfo){.load_op = SDL_GPU_LOADOP_CLEAR};

    renderer->quad3pline =
        FG_CreateQuad3Pline(renderer->device, renderer->window);
    if (!renderer->quad3pline) {
        SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
        SDL_DestroyGPUDevice(renderer->device);
        SDL_free(renderer);
        return NULL;
    }

    return renderer;
}

bool FG_RendererDraw(FG_Renderer    *renderer,
                     const FG_Quad3 *begin,
                     const FG_Quad3 *end)
{
    SDL_GPUCommandBuffer *cmdbuf =
        SDL_AcquireGPUCommandBuffer(renderer->device);
    if (!cmdbuf) return false;

    Uint32 width;
    Uint32 height;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, renderer->window,
                                               &renderer->target.texture,
                                               &width, &height)) {
        SDL_CancelGPUCommandBuffer(cmdbuf);
        return false;
    }

    if (renderer->target.texture) {
        SDL_GPUCopyPass *cpass = SDL_BeginGPUCopyPass(cmdbuf);
        FG_Mat4          projmat;
        FG_SetProjMat4(FG_DegToRad(60.0F), (float)width / (float)height,
                       &projmat);
        FG_Quad3PlineCopy(renderer->quad3pline, renderer->device, cpass,
                          &projmat, begin, end);
        SDL_EndGPUCopyPass(cpass);
        SDL_GPURenderPass *rpass =
            SDL_BeginGPURenderPass(cmdbuf, &renderer->target, 1, NULL);
        FG_Quad3PlineDraw(renderer->quad3pline, rpass, (Uint32)(end - begin));
        SDL_EndGPURenderPass(rpass);
    }

    return SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void FG_DestroyRenderer(FG_Renderer *renderer)
{
    FG_ReleaseQuad3Pline(renderer->device, renderer->quad3pline);
    SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
    SDL_DestroyGPUDevice(renderer->device);
    SDL_free(renderer);
}
