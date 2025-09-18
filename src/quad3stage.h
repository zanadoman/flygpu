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

#ifndef FLYGPU_QUAD3STAGE_H
#define FLYGPU_QUAD3STAGE_H

#include "../include/flygpu/flygpu.h"
#include "linalg.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <stdbool.h>

typedef struct FG_Quad3Stage FG_Quad3Stage;

FG_Quad3Stage * FG_CreateQuad3Stage(SDL_GPUDevice *device);

bool FG_Quad3StageCopy(FG_Quad3Stage               *self,
                       SDL_GPUCopyPass             *cpypass,
                       Uint32                       mask,
                       const FG_Mat4               *vpmat,
                       const FG_Quad3StageDrawInfo *info);

void FG_Quad3StageDraw(FG_Quad3Stage     *self,
                       SDL_GPURenderPass *rndrpass,
                       const FG_Material *fallback);

void FG_DestroyQuad3Stage(FG_Quad3Stage *self);

#endif /* FLYGPU_QUAD3STAGE_H */
