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

#ifndef FLYGPU_UTILS_H
#define FLYGPU_UTILS_H

#include "flygpu.h"

#include <SDL3/SDL_begin_code.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FG_PI 3.1415927410125732421875F

#define FG_DegsToRads(d) ((d) * FG_PI / 180.0F)

#define FG_RadsToDegs(r) ((r) * 180.0F / FG_PI)

SDL_DECLSPEC void SDLCALL FG_InitCamera(FG_Camera *camera);

SDL_DECLSPEC void SDLCALL FG_InitQuad3(FG_Quad3 *quad3);

SDL_DECLSPEC void SDLCALL FG_InitDirectLight(FG_DirectLight *direct);

SDL_DECLSPEC void SDLCALL FG_InitOmniLight(FG_OmniLight *omni);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#include <SDL3/SDL_close_code.h>

#endif /* FLYGPU_UTILS_H */
