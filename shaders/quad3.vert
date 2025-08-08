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

layout(location = 0)  in vec4 MODEL0;
layout(location = 1)  in vec4 MODEL1;
layout(location = 2)  in vec4 MODEL2;
layout(location = 3)  in vec4 MODEL3;
layout(location = 4)  in vec4 MVP0;
layout(location = 5)  in vec4 MVP1;
layout(location = 6)  in vec4 MVP2;
layout(location = 7)  in vec4 MVP3;
layout(location = 8)  in vec3 TBN0;
layout(location = 9)  in vec3 TBN1;
layout(location = 10) in vec3 TBN2;
layout(location = 11) in vec4 inColorTL;
layout(location = 12) in vec4 inColorBL;
layout(location = 13) in vec4 inColorBR;
layout(location = 14) in vec4 inColorTR;
layout(location = 15) in vec4 inTexCoord;

layout(location = 0) out mat3 TBN;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec4 fragColor;
layout(location = 5) out vec2 fragTexCoord;

const uint indices[6] = uint[](0, 1, 3, 1, 2, 3);

const vec4 positions[4] = vec4[](
    vec4(-0.5, 0.5, 0.0, 1.0),
    vec4(-0.5, -0.5, 0.0, 1.0),
    vec4(0.5, -0.5, 0.0, 1.0),
    vec4(0.5, 0.5, 0.0, 1.0)
);

void main()
{
    uint i       = indices[gl_VertexIndex];
    gl_Position  = mat4(MVP0, MVP1, MVP2, MVP3) * positions[i];
    TBN          = mat3(TBN0, TBN1, TBN2);
    fragPosition = (mat4(MODEL0, MODEL1, MODEL2, MODEL3) * positions[i]).xyz;
    fragColor    = vec4[](inColorTL, inColorBL, inColorBR, inColorTR)[i];
    fragTexCoord = vec2[](
        inTexCoord.xy,
        vec2(inTexCoord.x, inTexCoord.w),
        inTexCoord.zw,
        vec2(inTexCoord.z, inTexCoord.y)
    )[i];
}
