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

#include "../include/flygpu/linalg.h"

#include <SDL3/SDL_stdinc.h>

void FG_SetProjMat4(float fov, float aspect, FG_Mat4 *projmat)
{
    float focal = 1.0F / SDL_tanf(fov / 2.0F);

    *projmat = (FG_Mat4){
        .cols[0].x = focal / aspect,
        .cols[1].y = focal,
        .cols[2]   = {
            .z = (FG_FAR + FG_NEAR) / (FG_NEAR - FG_FAR),
            .w = -1.0F
        },
        .cols[3].z = 2.0F * FG_FAR * FG_NEAR / (FG_NEAR - FG_FAR)
    };
}

void FG_SetTransMat4(const FG_Transform3 *restrict transform3,
                     FG_Mat4             *restrict transmat)
{
    float cos = SDL_cosf(transform3->rotation);
    float sin = SDL_sinf(transform3->rotation);

    *transmat = (FG_Mat4){
        .cols[0]   = {
            .x = cos * transform3->scale.x,
            .y = sin * transform3->scale.x
        },
        .cols[1]   = {
            .x = -sin * transform3->scale.y,
            .y = cos * transform3->scale.y
        },
        .cols[2].z = 1.0F,
        .cols[3]   = {
            .x = transform3->translation.x,
            .y = transform3->translation.y,
            .z = transform3->translation.z,
            .w = 1.0F
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

    for (i = 0; i != FG_LINALG_DIMS_VEC4; ++i) {
        for (j = 0; j != FG_LINALG_DIMS_VEC4; ++j) {
            out->data[4 * j + i] = 0.0F;
            for (k = 0; k != FG_LINALG_DIMS_VEC4; ++k) {
                out->data[4 * j + i] += lhs->data[4 * k + i] * rhs->data[4 * j + k];
            }
        }
    }
}
