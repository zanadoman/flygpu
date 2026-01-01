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

struct DirectLight
{
    float3 Direction;
    uint   Padding0;
    float3 Color;
    uint   Padding1;
};

struct OmniLight
{
    float3 Position;
    float  Radius;
    float3 Color;
    uint   Padding0;
};

struct UniformBuffer
{
    float3 Origo;
    uint   DirectsSize;
    float3 Ambient;
    uint   OmnisSize;
    float3 Padding0;
    float  Shine;
};

Texture2D<float4>             tPosition : register(t0, space2);
SamplerState                  sPosition : register(s0, space2);
Texture2D<float4>             tNormal   : register(t1, space2);
SamplerState                  sNormal   : register(s1, space2);
Texture2D<float4>             tSpecular : register(t2, space2);
SamplerState                  sSpecular : register(s2, space2);
Texture2D<float4>             tAlbedo   : register(t3, space2);
SamplerState                  sAlbedo   : register(s3, space2);
ByteAddressBuffer             bDirects  : register(t4, space2);
ByteAddressBuffer             bOmnis    : register(t5, space2);
ConstantBuffer<UniformBuffer> cUniform  : register(b0, space3);

float4 main(const noperspective float2 TexCoord : TEXCOORD0) : SV_Target0
{
    float3 normal = tNormal.Sample(sNormal, TexCoord).xyz;
    if (all(normal == 0.0F)) discard;

                 normal   = normalize(normal);
    const float3 albedo   = tAlbedo.Sample(sAlbedo, TexCoord).rgb;
    float3       output   = albedo * cUniform.Ambient;
    const float3 specular = tSpecular.Sample(sSpecular, TexCoord).rgb;

    for (uint i = 0; i != cUniform.DirectsSize; i += sizeof(DirectLight)) {
        const DirectLight light = bDirects.Load<DirectLight>(i);
        const float       NdotL = dot(normal, normalize(-light.Direction));

        if (0.0F <= NdotL) {
            output += albedo * NdotL * light.Color
                    + specular * pow(NdotL, cUniform.Shine) * light.Color;
        }
    }

    const float3 position = tPosition.Sample(sPosition, TexCoord).xyz;
    const float3 viewDir  = normalize(cUniform.Origo - position);

    for (uint i = 0; i != cUniform.OmnisSize; i += sizeof(OmniLight)) {
        const OmniLight light    = bOmnis.Load<OmniLight>(i);
        float3          lightDir = light.Position - position;
        const float     distance = length(lightDir);

        if (light.Radius <= distance || distance == 0.0F) continue;

                    lightDir /= distance;
        const float NdotL     = dot(normal, lightDir);

        if (0.0F <= NdotL) {
            const float attenuation = distance / light.Radius;

            output += (albedo * NdotL * light.Color
                    + specular
                    * pow(
                          max(dot(normal, normalize(lightDir + viewDir)), 0.0F),
                          cUniform.Shine
                      )
                    * light.Color)
                    * (1.0F - attenuation * attenuation);
        }
    }

    return float4(output, 1.0F);
}
