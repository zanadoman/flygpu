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

#version 460

layout(location = 0) in vec4 mvp0;
layout(location = 1) in vec4 mvp1;
layout(location = 2) in vec4 mvp2;
layout(location = 3) in vec4 mvp3;
layout(location = 4) in vec4 inColorTL;
layout(location = 5) in vec4 inColorBL;
layout(location = 6) in vec4 inColorBR;
layout(location = 7) in vec4 inColorTR;
layout(location = 8) in vec2 inTexCoordTL;
layout(location = 9) in vec2 inTexCoordBR;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

const uint indices[6] = uint[](0, 1, 3, 1, 2, 3);

const vec2 positions[4] = vec2[](
    vec2(-0.5, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0.5, 0.5)
);

void main()
{
    uint i = indices[gl_VertexIndex];
    gl_Position  = mat4(mvp0, mvp1, mvp2, mvp3) * vec4(positions[i], 0.0, 1.0);
    fragColor    = vec4[](inColorTL, inColorBL, inColorBR, inColorTR)[i];
    fragTexCoord = vec2[](inTexCoordTL, vec2(inTexCoordTL.x, inTexCoordBR.y),
                          inTexCoordBR, vec2(inTexCoordBR.x, inTexCoordTL.y))[i];
}
