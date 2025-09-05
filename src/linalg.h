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

#ifndef FLYGPU_LINALG_H
#define FLYGPU_LINALG_H

#include "../include/flygpu/flygpu.h"

typedef struct
{
    float m[3 * 3];
} FG_Mat3;

typedef struct
{
    float m[4 * 4];
} FG_Mat4;

void FG_SetProjMat4(const FG_Perspective *perspective, float aspect, FG_Mat4 *projmat);

void FG_SetViewMat4(const FG_Transform3 *transform, FG_Mat4 *viewmat);

void FG_SetModelMat4(const FG_Transform3 *transform, FG_Mat4 *modelmat);

void FG_SetTBNMat3(float rotation, FG_Mat3 *tbnmat);

void FG_MulMat4s(const FG_Mat4 *lhs, const FG_Mat4 *rhs, FG_Mat4 *out);

#endif /* FLYGPU_LINALG_H */
