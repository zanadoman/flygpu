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
    nointerpolation float3 ColorTL  : TEXCOORD0;
    nointerpolation float3 ColorBL  : TEXCOORD1;
    nointerpolation float3 ColorBR  : TEXCOORD2;
    nointerpolation float3 ColorTR  : TEXCOORD3;
    noperspective   float2 TexCoord : TEXCOORD4;
};

Texture2D<float4> tTexture : register(t0, space2);
SamplerState      sTexture : register(s0, space2);

float4 main(const Input input) : SV_Target0
{
    float4 output = tTexture.Sample(sTexture, input.TexCoord);
    if (output.a <= 0.0F) discard;

    const float2 colorCoord  = frac(input.TexCoord);
    output.rgb              *= lerp(
        lerp(input.ColorTL, input.ColorTR, colorCoord.x),
        lerp(input.ColorBL, input.ColorBR, colorCoord.x),
        colorCoord.y
    );

    return output;
}
