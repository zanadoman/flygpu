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

#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>

SDL_GPUShader *FG_LoadShader(SDL_GPUDevice      *device,
                             const char         *path,
                             SDL_GPUShaderStage  stage,
                             Uint32              samplers)
{
    SDL_GPUShaderCreateInfo  info   = {
        .format       = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage        = stage,
        .num_samplers = samplers
    };
    void                    *code   = SDL_LoadFile(path, &info.code_size);
    SDL_GPUShader           *shader = NULL;

    if (!code) return NULL;

    info.code = code;

    shader = SDL_CreateGPUShader(device, &info);

    SDL_free(code);
    return shader;
}
