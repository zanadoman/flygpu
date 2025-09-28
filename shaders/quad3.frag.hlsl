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

struct Input
{
    nointerpolation float3x3 TBN      : TEXCOORD0;
    noperspective   float3   Position : TEXCOORD3;
    nointerpolation float3   ColorTL  : TEXCOORD4;
    nointerpolation float3   ColorBL  : TEXCOORD5;
    nointerpolation float3   ColorBR  : TEXCOORD6;
    nointerpolation float3   ColorTR  : TEXCOORD7;
    noperspective   float2   TexCoord : TEXCOORD8;
};

struct Output
{
    float4 Position : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Specular : SV_Target2;
    float4 Albedo   : SV_Target3;
};

Texture2D<float4> tAlbedo   : register(t0, space2);
SamplerState      sAlbedo   : register(s0, space2);
Texture2D<float4> tAlpha    : register(t1, space2);
SamplerState      sAlpha    : register(s1, space2);
Texture2D<float4> tSpecular : register(t2, space2);
SamplerState      sSpecular : register(s2, space2);
Texture2D<float4> tNormal   : register(t3, space2);
SamplerState      sNormal   : register(s3, space2);

Output main(const Input input)
{
    if (tAlpha.Sample(sAlpha, input.TexCoord).a <= 0.0F) discard;

    Output output;

    output.Position = float4(input.Position, 0.0F);
    output.Normal   = float4(
        mul(tNormal.Sample(sNormal, input.TexCoord).rgb * 2.0F - 1.0F, input.TBN),
        0.0F
    );

    const float2 colorCoord = frac(input.TexCoord);
    const float3 color      = lerp(
        lerp(input.ColorTL, input.ColorTR, colorCoord.x),
        lerp(input.ColorBL, input.ColorBR, colorCoord.x),
        colorCoord.y
    );

    output.Specular = float4(
        color * tSpecular.Sample(sSpecular, input.TexCoord).rgb, 0.0F);
    output.Albedo   = float4(
        color * tAlbedo.Sample(sAlbedo, input.TexCoord).rgb, 0.0F);

    return output;
}
