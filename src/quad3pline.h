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

#ifndef FLYGPU_QUAD3PLINE_H
#define FLYGPU_QUAD3PLINE_H

#include <SDL3/SDL_gpu.h>

#include "../include/flygpu/flygpu.h"
#include "../include/flygpu/linalg.h"

typedef struct FG_Quad3Pline FG_Quad3Pline;

FG_Quad3Pline *FG_CreateQuad3Pline(SDL_GPUDevice *device, SDL_Window *window);

bool FG_Quad3PlineCopy(FG_Quad3Pline   *self,
                       SDL_GPUCopyPass *copy_pass,
                       const FG_Mat4   *projmat,
                       const FG_Quad3  *begin,
                       const FG_Quad3  *end);

void FG_Quad3PlineDraw(FG_Quad3Pline *self, SDL_GPURenderPass *render_pass);

void FG_ReleaseQuad3Pline(FG_Quad3Pline *self);

#endif /* FLYGPU_QUAD3PLINE_H */
