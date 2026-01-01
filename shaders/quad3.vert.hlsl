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

struct Input
{
    float4x4 MODEL       : TEXCOORD0;
    float4x4 MVP         : TEXCOORD4;
    float3x3 TBN         : TEXCOORD8;
    float3   ColorTL     : TEXCOORD11;
    float3   ColorBL     : TEXCOORD12;
    float3   ColorBR     : TEXCOORD13;
    float3   ColorTR     : TEXCOORD14;
    float4   TexCoord    : TEXCOORD15;
    uint     VertexIndex : SV_VertexID;
};

struct Output
{
    float3x3 TBN            : TEXCOORD0;
    float3   Position       : TEXCOORD3;
    float3   ColorTL        : TEXCOORD4;
    float3   ColorBL        : TEXCOORD5;
    float3   ColorBR        : TEXCOORD6;
    float3   ColorTR        : TEXCOORD7;
    float2   TexCoord       : TEXCOORD8;
    float4   VertexPosition : SV_Position;
};

static const uint   INDICES[6]   = { 0, 1, 3, 1, 2, 3 };
static const float4 POSITIONS[4] = {
    float4(-0.5F, 0.5F, 0.0F, 1.0F),
    float4(-0.5F, -0.5F, 0.0F, 1.0F),
    float4(0.5F, -0.5F, 0.0F, 1.0F),
    float4(0.5F, 0.5F, 0.0F, 1.0F)
};

Output main(const Input input)
{
    Output output;

    output.TBN = input.TBN;

    const uint   i        = INDICES[input.VertexIndex];
    const float4 position = POSITIONS[i];

    output.Position = mul(position, input.MODEL).xyz;
    output.ColorTL  = input.ColorTL;
    output.ColorBL  = input.ColorBL;
    output.ColorBR  = input.ColorBR;
    output.ColorTR  = input.ColorTR;
    switch (i) {
    case 0: output.TexCoord = input.TexCoord.xy; break;
    case 1: output.TexCoord = input.TexCoord.xw; break;
    case 2: output.TexCoord = input.TexCoord.zw; break;
    case 3: output.TexCoord = input.TexCoord.zy; break;
    }
    output.VertexPosition = mul(position, input.MVP);

    return output;
}
