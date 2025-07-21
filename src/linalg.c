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
    float focal       = 1.0F / SDL_tanf(fov / 2.0F);
    projmat->data[0]  = focal / aspect;
    projmat->data[1]  = 0.0F;
    projmat->data[2]  = 0.0F;
    projmat->data[3]  = 0.0F;
    projmat->data[4]  = 0.0F;
    projmat->data[5]  = focal;
    projmat->data[6]  = 0.0F;
    projmat->data[7]  = 0.0F;
    projmat->data[8]  = 0.0F;
    projmat->data[9]  = 0.0F;
    projmat->data[10] = (FG_FAR + FG_NEAR) / (FG_NEAR - FG_FAR);
    projmat->data[11] = -1.0F;
    projmat->data[12] = 0.0F;
    projmat->data[13] = 0.0F;
    projmat->data[14] = 2.0F * FG_FAR * FG_NEAR / (FG_NEAR - FG_FAR);
    projmat->data[15] = 0.0F;
}

void FG_SetTransMat4(const FG_Transform3 *transform3, FG_Mat4 *transmat)
{
    float cos          = SDL_cosf(transform3->rotation);
    float sin          = SDL_sinf(transform3->rotation);
    transmat->data[0]  = cos * transform3->scale.x;
    transmat->data[1]  = sin * transform3->scale.x;
    transmat->data[2]  = 0.0F;
    transmat->data[3]  = 0.0F;
    transmat->data[4]  = -sin * transform3->scale.y;
    transmat->data[5]  = cos * transform3->scale.y;
    transmat->data[6]  = 0.0F;
    transmat->data[7]  = 0.0F;
    transmat->data[8]  = 0.0F;
    transmat->data[9]  = 0.0F;
    transmat->data[10] = 1.0F;
    transmat->data[11] = 0.0F;
    transmat->data[12] = transform3->translation.x;
    transmat->data[13] = transform3->translation.y;
    transmat->data[14] = transform3->translation.z;
    transmat->data[15] = 1.0F;
}

void FG_MulMat4s(const FG_Mat4 *restrict lhs,
                 const FG_Mat4 *restrict rhs,
                 FG_Mat4 *restrict       out)
{
    Sint32 i = 0;
    Sint32 j = 0;
    Sint32 k = 0;

    for (i = 0; i != FG_LINALG_DIMS_VEC4; ++i) {
        for (j = 0; j != FG_LINALG_DIMS_VEC4; ++j) {
            out->data[j * 4 + i] = 0.0F;
            for (k = 0; k != FG_LINALG_DIMS_VEC4; ++k) {
                out->data[j * 4 + i] += lhs->data[k * 4 + i]
                                      * rhs->data[j * 4 + k];
            }
        }
    }
}
