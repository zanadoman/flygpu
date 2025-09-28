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

#include "../include/flygpu/flygpu.h"
#include "linalg.h"

#include <SDL3/SDL_stdinc.h>

void FG_SetProjMat4(const FG_Perspective *perspective, float aspect, FG_Mat4 *projmat)
{
    float focal = 1.0F / SDL_tanf(perspective->fov * 0.5F);

    *projmat = (FG_Mat4){
        .m = {
            [0]  = focal / aspect,
            [5]  = focal,
            [10] = (perspective->far + perspective->near)
                 / (perspective->near - perspective->far),
            [11] = -1.0F,
            [14] = 2.0F * perspective->far * perspective->near
                 / (perspective->near - perspective->far)
        }
    };
}

void FG_SetViewMat4(const FG_Transform3 *restrict transf, FG_Mat4 *restrict viewmat)
{
    float cos = SDL_cosf(transf->rotation);
    float sin = SDL_sinf(transf->rotation);

    *viewmat = (FG_Mat4){
        .m = {
            [0]  = cos * transf->scale.x,
            [1]  = -sin * transf->scale.x,
            [4]  = sin * transf->scale.y,
            [5]  = cos * transf->scale.y,
            [10] = 1.0F,
            [12] = transf->transl.x * -cos - transf->transl.y * sin,
            [13] = transf->transl.x * sin - transf->transl.y * cos,
            [14] = -transf->transl.z,
            [15] = 1.0F
        }
    };
}

void FG_SetModelMat4(const FG_Transform3 *restrict transf, FG_Mat4 *restrict modelmat)
{
    float cos = SDL_cosf(transf->rotation);
    float sin = SDL_sinf(transf->rotation);

    *modelmat = (FG_Mat4){
        .m = {
            [0]  = cos * transf->scale.x,
            [1]  = sin * transf->scale.x,
            [4]  = -sin * transf->scale.y,
            [5]  = cos * transf->scale.y,
            [10] = 1.0F,
            [12] = transf->transl.x,
            [13] = transf->transl.y,
            [14] = transf->transl.z,
            [15] = 1.0F
        }
    };
}

void FG_SetTBNMat3(float rotation, FG_Mat3 *tbnmat)
{
    float cos = SDL_cosf(rotation);
    float sin = SDL_sinf(rotation);

    *tbnmat = (FG_Mat3){
        .m = {
            [0] = cos,
            [1] = sin,
            [3] = -sin,
            [4] = cos,
            [8] = 1.0F
        }
    };
}

void FG_MulMat4s(const FG_Mat4 *restrict lhs,
                 const FG_Mat4 *restrict rhs,
                 FG_Mat4       *restrict out)
{
    Uint8 i = 0;
    Uint8 j = 0;
    Uint8 k = 0;

    for (i = 0; i != 4; ++i) {
        for (j = 0; j != 4; ++j) {
            out->m[j * 4 + i] = 0.0F;
            for (k = 0; k != 4; ++k) {
                out->m[j * 4 + i] += lhs->m[k * 4 + i] * rhs->m[j * 4 + k];
            }
        }
    }
}
