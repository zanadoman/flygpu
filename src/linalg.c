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

void FG_SetProjMat4(float fov, float aspect, FG_Mat4 *out)
{
    float f    = 1.0F / SDL_tanf(fov / 2.0F);
    out->m[0]  = f / aspect;
    out->m[1]  = 0.0F;
    out->m[2]  = 0.0F;
    out->m[3]  = 0.0F;
    out->m[4]  = 0.0F;
    out->m[5]  = f;
    out->m[6]  = 0.0F;
    out->m[7]  = 0.0F;
    out->m[8]  = 0.0F;
    out->m[9]  = 0.0F;
    out->m[10] = (FG_FAR + FG_NEAR) / (FG_NEAR - FG_FAR);
    out->m[11] = -1.0F;
    out->m[12] = 0.0F;
    out->m[13] = 0.0F;
    out->m[14] = 2.0F * FG_FAR * FG_NEAR / (FG_NEAR - FG_FAR);
    out->m[15] = 0.0F;
}

void FG_SetTransMat4(const FG_Transform3 *src, FG_Mat4 *out)
{
    float c    = SDL_cosf(src->r);
    float s    = SDL_sinf(src->r);
    out->m[0]  = c * src->s.x;
    out->m[1]  = s * src->s.x;
    out->m[2]  = 0.0F;
    out->m[3]  = 0.0F;
    out->m[4]  = -s * src->s.y;
    out->m[5]  = c * src->s.y;
    out->m[6]  = 0.0F;
    out->m[7]  = 0.0F;
    out->m[8]  = 0.0F;
    out->m[9]  = 0.0F;
    out->m[10] = 1.0F;
    out->m[11] = 0.0F;
    out->m[12] = src->t.x;
    out->m[13] = src->t.y;
    out->m[14] = src->t.z;
    out->m[15] = 1.0F;
}

void FG_MulMat4s(const FG_Mat4 *restrict lhs,
                 const FG_Mat4 *restrict rhs,
                 FG_Mat4 *restrict out)
{
    for (Sint32 i = 0; i != FG_LINALG_DIMS_VEC4; ++i) {
        for (Sint32 j = 0; j != FG_LINALG_DIMS_VEC4; ++j) {
            out->m[j * 4 + i] = 0.0F;
            for (Sint32 k = 0; k != FG_LINALG_DIMS_VEC4; ++k) {
                out->m[j * 4 + i] += lhs->m[k * 4 + i] * rhs->m[j * 4 + k];
            }
        }
    }
}
