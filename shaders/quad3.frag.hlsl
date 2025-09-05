Texture2D    albedoTexture   : register(t0, space2);
SamplerState albedoSampler   : register(s0, space2);
Texture2D    alphaTexture    : register(t1, space2);
SamplerState alphaSampler    : register(s1, space2);
Texture2D    specularTexture : register(t2, space2);
SamplerState specularSampler : register(s2, space2);
Texture2D    normalTexture   : register(t3, space2);
SamplerState normalSampler   : register(s3, space2);

struct FSInput
{
    nointerpolation float3x3 TBN      : TEXCOORD0;
    noperspective   float3   Position : TEXCOORD3;
    nointerpolation float3   ColorTL  : TEXCOORD4;
    nointerpolation float3   ColorBL  : TEXCOORD5;
    nointerpolation float3   ColorBR  : TEXCOORD6;
    nointerpolation float3   ColorTR  : TEXCOORD7;
    noperspective   float2   TexCoord : TEXCOORD8;
};

struct FSOutput
{
    float4 Position : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Specular : SV_Target2;
    float4 Albedo   : SV_Target3;
};

[shader("pixel")]
FSOutput fragMain(const FSInput input)
{
    FSOutput output;

    if (alphaTexture.Sample(alphaSampler, input.TexCoord).a <= 0.0F) discard;

    output.Position = float4(input.Position, 0.0F);
    output.Normal   = float4(
        mul(
            normalTexture.Sample(normalSampler, input.TexCoord).rgb * 2.0F - 1.0F,
            input.TBN
        ),
        0.0F
    );

    float2 colorCoord = frac(input.TexCoord);
    float3 color      = lerp(
        lerp(input.ColorTL, input.ColorTR, colorCoord.x),
        lerp(input.ColorBL, input.ColorBR, colorCoord.x),
        colorCoord.y
    );

    output.Specular = float4(
        color * specularTexture.Sample(specularSampler, input.TexCoord).rgb, 0.0F);
    output.Albedo   = float4(
        color * albedoTexture.Sample(albedoSampler, input.TexCoord).rgb, 0.0F);

    return output;
}
