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

#ifndef FLYGPU_SHADING_STAGE_H
#define FLYGPU_SHADING_STAGE_H

#include "../include/flygpu/flygpu.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stdbool.h>

typedef struct FG_ShadingStage FG_ShadingStage;

FG_ShadingStage * FG_CreateShadingStage(SDL_GPUDevice        *device,
                                        SDL_GPUTextureFormat  targbuf_fmt);

void FG_ShadingStageUpdate(FG_ShadingStage        *self,
                           SDL_GPUColorTargetInfo *gbuftarg_infos);

bool FG_ShadingStageCopy(FG_ShadingStage               *self,
                         SDL_GPUCopyPass               *cpypass,
                         Uint32                         mask,
                         const FG_ShadingStageDrawInfo *info);

void FG_ShadingStageDraw(FG_ShadingStage      *self,
                         SDL_GPUCommandBuffer *cmdbuf,
                         SDL_GPURenderPass    *rndrpass,
                         const FG_Camera      *camera);

void FG_DestroyShadingStage(FG_ShadingStage *self);

#endif /* FLYGPU_SHADING_STAGE_H */
