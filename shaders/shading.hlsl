struct DirectLight
{
    float3 Direction;
    float3 Color;
};

struct OmniLight
{
    float3 Position;
    float  Radius;
    float3 Color;
};

Texture2D    positionTexture : register(t0, space2);
SamplerState positionSampler : register(s0, space2);
Texture2D    normalTexture   : register(t1, space2);
SamplerState normalSampler   : register(s1, space2);
Texture2D    specularTexture : register(t2, space2);
SamplerState specularSampler : register(s2, space2);
Texture2D    albedoTexture   : register(t3, space2);
SamplerState albedoSampler   : register(s3, space2);

StructuredBuffer<DirectLight> directBuffer : register(t4, space2);
StructuredBuffer<OmniLight>   omniBuffer   : register(t5, space2);

struct UBO
{
    float3 Origo;
    uint   DirectCount;
    uint   OmniCount;
};
ConstantBuffer<UBO> ubo : register(b0, space3);

static float3 normal;
static float3 output;
static float3 specular;
static float3 albedo;
static float3 viewDir;
static float  attenuation;
static float3 lightDir;
static float  NdotL;

void accumulate(const float3 color)
{
    output += (albedo
            * NdotL
            * color
            + specular
            * pow(max(dot(normal, normalize(lightDir + viewDir)), 0.0F), 32.0F)
            * color)
            * attenuation;
}

[shader("pixel")]
float4 fragMain(const noperspective float2 TexCoord : TEXCOORD0) : SV_Target0
{
    normal = normalTexture.Sample(normalSampler, TexCoord).xyz;
    if (all(normal == 0.0F.xxx)) discard;

    output = 0.0F.xxx;

    const float3 position = positionTexture.Sample(positionSampler, TexCoord).xyz;
                 normal   = normalize(normal);
                 specular = specularTexture.Sample(specularSampler, TexCoord).rgb;
                 albedo   = albedoTexture.Sample(albedoSampler, TexCoord).rgb;
                 viewDir  = normalize(ubo.Origo - position);

    attenuation = 1.0F;

    for (uint i = 0; i != ubo.DirectCount; ++i) {
        lightDir = normalize(-directBuffer[i].Direction);

        NdotL = dot(normal, lightDir);
        if (0.0F <= NdotL) accumulate(directBuffer[i].Color);
    }

    for (uint i = 0; i != ubo.OmniCount; ++i) {
        lightDir = omniBuffer[i].Position - position;

        const float distance = length(lightDir);
        if (omniBuffer[i].Radius <= distance || !distance) continue;

        lightDir /= distance;

        NdotL = dot(normal, lightDir);
        if (NdotL < 0.0F) continue;

        attenuation = distance / omniBuffer[i].Radius;
        attenuation = 1.0F - attenuation * attenuation;

        accumulate(omniBuffer[i].Color);
    }

    return float4(output, 0.0F);
}
