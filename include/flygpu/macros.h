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

#ifndef FLYGPU_MACROS_H
#define FLYGPU_MACROS_H

#include "flygpu.h" /* IWYU pragma: keep */

#define FG_PI 3.1415927410125732421875F

#define FG_DegsToRads(d) ((d) * FG_PI / 180.0F)

#define FG_RadsToDegs(r) ((r) * 180.0F / FG_PI)

#define FG_DEF_CAMERA (FG_Camera){                       \
    .viewport.br  = { .x = 1.0F, .y = 1.0F },            \
    .perspective  = {                                    \
        .fov  = 0.927295207977294921875F,                \
        .near = 0.1F,                                    \
        .far  = 1000.0F                                  \
    },                                                   \
    .ambient      = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
    .transf.scale = { .x = 1.0F, .y = 1.0F },            \
    .mask         = 0xFFFFFFFF                           \
}

#define FG_DEF_QUAD3 (FG_Quad3){                   \
    .transf.scale = { .x = 1.0F, .y = 1.0F },      \
    .color        = {                              \
        .tl = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .bl = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .br = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
        .tr = { .x = 1.0F, .y = 1.0F, .z = 1.0F }  \
    },                                             \
    .coords.br    = { .x = 1.0F, .y = 1.0F },      \
    .mask         = 0xFFFFFFFF                     \
}

#define FG_DEF_DIRECT_LIGHT (FG_DirectLight){           \
    .direction.z = -1.0F,                               \
    .color       = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
    .mask        = 0xFFFFFFFF                           \
}

#define FG_DEF_OMNI_LIGHT (FG_OmniLight){          \
    .radius = 10.0F,                               \
    .color  = { .x = 1.0F, .y = 1.0F, .z = 1.0F }, \
    .mask   = 0xFFFFFFFF                           \
}

#endif /* FLYGPU_MACROS_H */
