Texture2D    albedoTexture   : register(t0, space2);
SamplerState albedoSampler   : register(s0, space2);
Texture2D    alphaTexture    : register(t1, space2);
SamplerState alphaSampler    : register(s1, space2);
Texture2D    specularTexture : register(t2, space2);
SamplerState specularSampler : register(s2, space2);
Texture2D    normalTexture   : register(t3, space2);
SamplerState normalSampler   : register(s3, space2);

struct VSInput
{
    uint     VertexIndex : SV_VertexID;
    float4x4 MODEL       : TEXCOORD0;
    float4x4 MVP         : TEXCOORD4;
    float3x3 TBN         : TEXCOORD8;
    float3   ColorTL     : TEXCOORD11;
    float3   ColorBL     : TEXCOORD12;
    float3   ColorBR     : TEXCOORD13;
    float3   ColorTR     : TEXCOORD14;
    float4   TexCoord    : TEXCOORD15;
};

struct VSOutput
{
                    float4   VertexPosition : SV_Position;
    nointerpolation float3x3 TBN            : TEXCOORD0;
    noperspective   float3   Position       : TEXCOORD3;
    nointerpolation float3   ColorTL        : TEXCOORD4;
    nointerpolation float3   ColorBL        : TEXCOORD5;
    nointerpolation float3   ColorBR        : TEXCOORD6;
    nointerpolation float3   ColorTR        : TEXCOORD7;
    noperspective   float2   TexCoord       : TEXCOORD8;
};

struct FSOutput
{
    float4 Position : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Specular : SV_Target2;
    float4 Albedo   : SV_Target3;
};

static const uint INDICES[6] = { 0, 1, 3, 1, 2, 3 };

static const float4 POSITIONS[4] = {
    float4(-0.5F, 0.5F, 0.0F, 1.0F),
    float4(-0.5F, -0.5F, 0.0F, 1.0F),
    float4(0.5F, -0.5F, 0.0F, 1.0F),
    float4(0.5F, 0.5F, 0.0F, 1.0F)
};

[shader("vertex")]
VSOutput vertMain(const VSInput input)
{
    VSOutput   output;
    const uint i      = INDICES[input.VertexIndex];

    output.VertexPosition = mul(POSITIONS[i], input.MVP);
    output.TBN            = input.TBN;
    output.Position       = mul(POSITIONS[i], input.MODEL).xyz;
    output.ColorTL        = input.ColorTL;
    output.ColorBL        = input.ColorBL;
    output.ColorBR        = input.ColorBR;
    output.ColorTR        = input.ColorTR;
    switch (i) {
    case 0: output.TexCoord = input.TexCoord.xy; break;
    case 1: output.TexCoord = input.TexCoord.xw; break;
    case 2: output.TexCoord = input.TexCoord.zw; break;
    case 3: output.TexCoord = input.TexCoord.zy; break;
    }

    return output;
}

[shader("pixel")]
FSOutput fragMain(const VSOutput input)
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
