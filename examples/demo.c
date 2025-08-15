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
#include "../include/flygpu/linalg.h"

#include <SDL3/SDL_stdinc.h>

#include <stddef.h>

#define USE_MATERIALS

typedef enum
{
    MATERIAL_HOUSE,
    MATERIAL_LEATHER,
    MATERIAL_PINE,
    MATERIAL_COUNT
} Material;

static const char *MATERIAL_INFOS[MATERIAL_COUNT][3] = {
    { "./assets/albedos/big-house.png", NULL, NULL },
    {
        "./assets/albedos/block-big.png",
        "./assets/speculars/example.png",
        "./assets/normals/leather.png"
    },
    { "./assets/albedos/pine.png", NULL, NULL }
};

#include "common.h"

Sint32 main(void)
{
    INFO.camera_count               = 2;
    INFO.quad3_info.count           = 4;
    INFO.shading_info.ambient_count = 2;

    Init();
    InitMaterials();

    CAMERAS[0].viewport.br.x = 0.5F;
    CAMERAS[0].mask          = 1;

    CAMERAS[1].viewport.tl.x = 0.5F;
    CAMERAS[1].mask          = 2;

    QUAD3S[0].transform.translation.z = -1.0F;
    QUAD3S[0].transform.scale         = (FG_Vec2){ 0.5525F, 0.468F };
    QUAD3S[0].material                = &MATERIALS[MATERIAL_HOUSE];

    QUAD3S[1].transform = (FG_Transform3){
        .translation = { 2.0F, 2.0F, -10.0F },
        .rotation    = FG_DegsToRads(-45.0F),
        .scale       = { 3.0F, 1.0F }
    };
    QUAD3S[1].color     = (FG_QuadColor){
        { 1.0F, 1.0F, 1.0F },
        { 1.0F, 0.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 0.0F, 1.0F }
    };
    QUAD3S[1].material  = &MATERIALS[MATERIAL_LEATHER];
    QUAD3S[1].coords.br = (FG_Vec2){ 10.0F, 5.0F };

    QUAD3S[2].transform.translation = (FG_Vec3){ -2.0F, 0.0F, -5.0F };
    QUAD3S[2].transform.scale       = (FG_Vec2){ 1.23F, 1.95F };
    QUAD3S[2].material              = &MATERIALS[MATERIAL_PINE];
    QUAD3S[2].coords                = (FG_Rect){ { 0.0F, 1.0F }, { 1.0F, 0.0F } };

    QUAD3S[3].transform.translation = (FG_Vec3){ 0.0F, 0.3F, -1.0F };
    QUAD3S[3].transform.scale       = (FG_Vec2){ 0.5F, 0.05F };
    QUAD3S[3].color                 = (FG_QuadColor){
        { 1.0F, 0.0F, 0.0F },
        { 1.0F, 0.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F }
    };

    AMBIENTS[0].color     = (FG_Vec3){ 0.3F, 0.3F, 0.3F };
    AMBIENTS[0].mask      = 1;

    AMBIENTS[1].direction = (FG_Vec3){ 1.0F, 1.0F, -1.0F };
    AMBIENTS[1].mask      = 2;

    loop {
        MoveCamera(0, 0.001F, 0.0025F, D, A, SPACE, LSHIFT, S, W, Q, E);
        MoveCamera(1, 0.001F, 0.0025F, L, J, UP, DOWN, K, I, U, O);
        Update();
    }

    DestroyMaterials();
    Quit();
}
