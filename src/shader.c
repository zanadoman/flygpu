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

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_platform_defines.h> /* IWYU pragma: keep */
#include <SDL3/SDL_stdinc.h>

#include <stddef.h>

#ifdef SDL_PLATFORM_WINDOWS
#define FG_SHADER_DIR "shaders\\"
#else /* SDL_PLATFORM_WINDOWS */
#define FG_SHADER_DIR "shaders/"
#endif /* SDL_PLATFORM_WINDOWS */

#define FG_SHADER_SPIRV ".spv"
#define FG_SHADER_DXIL  ".dxil"

SDL_GPUShader * FG_LoadShader(SDL_GPUDevice      *device,
                              const char         *name,
                              SDL_GPUShaderStage  stage,
                              Uint32              samplers,
                              Uint32              ssbos,
                              Uint32              ubos)
{
    const char              *base   = SDL_GetBasePath();
    char                     path[
        (base ? SDL_strlen(base) : 0)
        + SDL_arraysize(FG_SHADER_DIR)
        + SDL_strlen(name)
        + SDL_max(SDL_arraysize(FG_SHADER_SPIRV), SDL_arraysize(FG_SHADER_DXIL))
    ];
    SDL_GPUShaderCreateInfo  info   = {
        .stage               = stage,
        .num_samplers        = samplers,
        .num_storage_buffers = ssbos,
        .num_uniform_buffers = ubos
    };
    size_t                   i      = 0;
    SDL_GPUShaderFormat      format = SDL_GetGPUShaderFormats(device);
    void                    *code   = NULL;
    SDL_GPUShader           *shader = NULL;

    *path = 0;

    if (base) i = SDL_strlcat(path, base, SDL_arraysize(path));
    i = SDL_strlcat(path + i, FG_SHADER_DIR, SDL_arraysize(path) - i);
    i = SDL_strlcat(path + i, name, SDL_arraysize(path) - i);
    if ((format & SDL_GPU_SHADERFORMAT_SPIRV) == SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_strlcat(path + i, FG_SHADER_SPIRV, SDL_arraysize(path) - i);
        info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    }
    else if ((format & SDL_GPU_SHADERFORMAT_DXIL) == SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_strlcat(path + i, FG_SHADER_DXIL, SDL_arraysize(path) - i);
        info.format = SDL_GPU_SHADERFORMAT_DXIL;
    }
    else {
        SDL_SetError("FlyGPU: Unsupported shader format!");
        return NULL;
    }

    code = SDL_LoadFile(path, &info.code_size);
    if (!code) return NULL;

    info.code = code;

    shader = SDL_CreateGPUShader(device, &info);

    SDL_free(code);
    return shader;
}
