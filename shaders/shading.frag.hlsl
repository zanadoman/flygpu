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

Texture2D<float4> positionTexture : register(t0, space2);
SamplerState      positionSampler : register(s0, space2);
Texture2D<float4> normalTexture   : register(t1, space2);
SamplerState      normalSampler   : register(s1, space2);
Texture2D<float4> specularTexture : register(t2, space2);
SamplerState      specularSampler : register(s2, space2);
Texture2D<float4> albedoTexture   : register(t3, space2);
SamplerState      albedoSampler   : register(s3, space2);

ByteAddressBuffer directBuffer : register(t4, space2);
ByteAddressBuffer omniBuffer   : register(t5, space2);

struct UBO
{
    float3 Ambient;
    uint   DirectSize;
    float3 Origo;
    uint   OmniSize;
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

float4 main(const noperspective float2 TexCoord : TEXCOORD0) : SV_Target0
{
    normal = normalTexture.Sample(normalSampler, TexCoord).xyz;
    if (all(normal == 0.0F.xxx)) discard;

    const float3 position = positionTexture.Sample(positionSampler, TexCoord).xyz;
                 normal   = normalize(normal);
                 specular = specularTexture.Sample(specularSampler, TexCoord).rgb;
                 albedo   = albedoTexture.Sample(albedoSampler, TexCoord).rgb;
                 viewDir  = normalize(ubo.Origo - position);

    output = albedo * ubo.Ambient;

    attenuation = 1.0F;

    for (uint i = 0; i != ubo.DirectSize; i += sizeof(DirectLight)) {
        const DirectLight light = directBuffer.Load<DirectLight>(i);

        lightDir = normalize(-light.Direction);

        NdotL = dot(normal, lightDir);
        if (0.0F <= NdotL) accumulate(light.Color);
    }

    for (uint i = 0; i != ubo.OmniSize; i += sizeof(OmniLight)) {
        const OmniLight light = omniBuffer.Load<OmniLight>(i);

        lightDir = light.Position - position;

        const float distance = length(lightDir);
        if (light.Radius <= distance || !distance) continue;

        lightDir /= distance;

        NdotL = dot(normal, lightDir);
        if (NdotL < 0.0F) continue;

        attenuation = distance / light.Radius;
        attenuation = 1.0F - attenuation * attenuation;

        accumulate(light.Color);
    }

    return float4(output, 0.0F);
}
