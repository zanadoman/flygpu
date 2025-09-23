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

#include "../include/flygpu/utils.h"

#include "../include/flygpu/flygpu.h"

void FG_InitCamera(FG_Camera *camera)
{
    *camera = (FG_Camera){
        .viewport.br     = { .x = 1.0F, .y = 1.0F },
        .perspective     = {
            .fov  = FG_DegsToRads(90.0F),
            .near = 0.1F,
            .far  = 1000.0F
        },
        .ambient         = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
        .transform.scale = { .x = 1.0F, .y = 1.0F },
        .mask            = 0xFFFFFFFF
    };
}

void FG_InitQuad3(FG_Quad3 *quad3)
{
    *quad3 = (FG_Quad3){
        .transform.scale = { .x = 1.0F, .y = 1.0F },
        .color           = {
            .tl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .bl = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .br = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
            .tr = { .x = 1.0F, .y = 1.0F, .z = 1.0F }
        },
        .coords.br       = { .x = 1.0F, .y = 1.0F },
        .mask            = 0xFFFFFFFF
    };
}

void FG_InitDirectLight(FG_DirectLight *direct)
{
    *direct = (FG_DirectLight){
        .direction.z = -1.0F,
        .color       = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
        .mask        = 0xFFFFFFFF
    };
}

void FG_InitOmniLight(FG_OmniLight *omni)
{
    *omni = (FG_OmniLight){
        .radius = 10.0F,
        .color  = { .x = 1.0F, .y = 1.0F, .z = 1.0F },
        .mask   = 0xFFFFFFFF
    };
}
