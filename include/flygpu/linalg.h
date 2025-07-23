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

#include <SDL3/SDL_stdinc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FG_FAR  100.0F
#define FG_NEAR 0.1F
#define FG_PI   3.14159F

#define FG_DegToRad(d) ((d) / 180.0F * FG_PI)
#define FG_RadToDeg(r) ((r) * 180.0F / FG_PI)

typedef struct
{
    float x;
    float y;
} FG_Vec2;

typedef struct
{
    float x;
    float y;
    float z;
} FG_Vec3;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} FG_Vec4;

typedef enum
{
    FG_LINALG_DIMS_VEC2 = sizeof(FG_Vec2) / sizeof(float),
    FG_LINALG_DIMS_VEC3 = sizeof(FG_Vec3) / sizeof(float),
    FG_LINALG_DIMS_VEC4 = sizeof(FG_Vec4) / sizeof(float),
    FG_LINALG_DIMS_MAT4 = FG_LINALG_DIMS_VEC4 * FG_LINALG_DIMS_VEC4
} FG_LinalgDims;

typedef union
{
    FG_Vec4 cols[FG_LINALG_DIMS_VEC4];
    float   data[FG_LINALG_DIMS_MAT4];
} FG_Mat4;

typedef struct
{
    FG_Vec3 translation;
    float   rotation;
    FG_Vec2 scale;
} FG_Transform3;

void FG_SetProjMat4(float fov, float aspect, FG_Mat4 *projmat);

void FG_SetTransMat4(const FG_Transform3 *transform3, FG_Mat4 *transmat);

void FG_MulMat4s(const FG_Mat4 *lhs, const FG_Mat4 *rhs, FG_Mat4 *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FLYGPU_LINALG_H */
