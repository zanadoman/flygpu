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

struct UniformBuffer
{
    float4x4 MATRIX;
    float3   ColorTL;
    uint     Padding0;
    float3   ColorBL;
    uint     Padding1;
    float3   ColorBR;
    uint     Padding2;
    float3   ColorTR;
    uint     Padding3;
    float4   TexCoord;
};

struct Output
{
    float3 ColorTL        : TEXCOORD0;
    float3 ColorBL        : TEXCOORD1;
    float3 ColorBR        : TEXCOORD2;
    float3 ColorTR        : TEXCOORD3;
    float2 TexCoord       : TEXCOORD4;
    float4 VertexPosition : SV_Position;
};

ConstantBuffer<UniformBuffer> cUniform : register(b0, space1);

static const uint   INDICES[6]   = { 0, 1, 3, 1, 2, 3 };
static const float4 POSITIONS[4] = {
    float4(-1.0F, 1.0F, 0.0F, 1.0F),
    float4(-1.0F, -1.0F, 0.0F, 1.0F),
    float4(1.0F, -1.0F, 0.0F, 1.0F),
    float4(1.0F, 1.0F, 0.0F, 1.0F)
};

Output main(const uint VertexIndex : SV_VertexID)
{
    Output output;

    output.ColorTL = cUniform.ColorTL;
    output.ColorBL = cUniform.ColorBL;
    output.ColorBR = cUniform.ColorBR;
    output.ColorTR = cUniform.ColorTR;

    const uint i = INDICES[VertexIndex];

    switch (i) {
    case 0: output.TexCoord = cUniform.TexCoord.xy; break;
    case 1: output.TexCoord = cUniform.TexCoord.xw; break;
    case 2: output.TexCoord = cUniform.TexCoord.zw; break;
    case 3: output.TexCoord = cUniform.TexCoord.zy; break;
    }
    output.VertexPosition = mul(POSITIONS[i], cUniform.MATRIX);

    return output;
}
