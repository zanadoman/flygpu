/*
  FlyGPU
  Copyright (C) 2025-2026 Domán Zana

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

struct Output
{
    float2 TexCoord       : TEXCOORD0;
    float4 VertexPosition : SV_Position;
};

static const float2 TEXCOORDS[3] = {
    float2(0.0F, 0.0F),
    float2(0.0F, 2.0F),
    float2(2.0F, 0.0F)
};
static const float4 POSITIONS[3] = {
    float4(-1.0F, 1.0F, 0.0F, 1.0F),
    float4(-1.0F, -3.0F, 0.0F, 1.0F),
    float4(3.0F, 1.0F, 0.0F, 1.0F)
};

Output main(const uint VertexIndex : SV_VertexID)
{
    const Output output = { TEXCOORDS[VertexIndex], POSITIONS[VertexIndex] };

    return output;
}
