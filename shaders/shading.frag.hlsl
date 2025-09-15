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
    float3 Ambient;
    uint   DirectsSize;
    float3 Origo;
    uint   OmnisSize;
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

static float3 Normal;
static float3 Specular;
static float3 Albedo;
static float3 Output;
static float3 ViewDir;
static float  Attenuation;
static float3 LightDir;
static float  NdotL;

void accumulate(const float3 color)
{
    Output += (Albedo
            * NdotL
            * color
            + Specular
            * pow(max(dot(Normal, normalize(LightDir + ViewDir)), 0.0F), 32.0F)
            * color)
            * Attenuation;
}

float4 main(const noperspective float2 TexCoord : TEXCOORD0) : SV_Target0
{
    Normal = tNormal.Sample(sNormal, TexCoord).xyz;
    if (all(Normal == 0.0F)) discard;

    const float3 position = tPosition.Sample(sPosition, TexCoord).xyz;
                 Normal   = normalize(Normal);
                 Specular = tSpecular.Sample(sSpecular, TexCoord).rgb;
                 Albedo   = tAlbedo.Sample(sAlbedo, TexCoord).rgb;
                 Output   = Albedo * cUniform.Ambient;
                 ViewDir  = normalize(cUniform.Origo - position);

    Attenuation = 1.0F;

    for (uint i = 0; i != cUniform.DirectsSize; i += sizeof(DirectLight)) {
        const DirectLight light = bDirects.Load<DirectLight>(i);

        LightDir = normalize(-light.Direction);

        NdotL = dot(Normal, LightDir);
        if (0.0F <= NdotL) accumulate(light.Color);
    }

    for (uint i = 0; i != cUniform.OmnisSize; i += sizeof(OmniLight)) {
        const OmniLight light = bOmnis.Load<OmniLight>(i);

        LightDir = light.Position - position;

        const float distance = length(LightDir);
        if (light.Radius <= distance || !distance) continue;

        LightDir /= distance;

        NdotL = dot(Normal, LightDir);
        if (NdotL < 0.0F) continue;

        Attenuation = distance / light.Radius;
        Attenuation = 1.0F - Attenuation * Attenuation;

        accumulate(light.Color);
    }

    return float4(Output, 0.0F);
}
