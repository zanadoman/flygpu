#version 460

struct AmbientLight
{
    vec3 direction;
    uint padding0;
    vec3 color;
    uint padding1;
};

struct OmniLight
{
    vec3  position;
    float radius;
    vec3  color;
    uint  padding0;
};

layout(set = 2, binding = 0) uniform sampler2D positionSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;
layout(set = 2, binding = 2) uniform sampler2D specularSampler;
layout(set = 2, binding = 3) uniform sampler2D albedoSampler;

layout(set = 2, binding = 4, std140) readonly buffer AmbientBuffer
{
    AmbientLight lights[];
} ambient;

layout(set = 2, binding = 5, std140) readonly buffer OmniBuffer
{
    OmniLight lights[];
} omni;

layout(set = 3, binding = 0, std140) uniform UniformBufferObject
{
    vec3 origo;
    uint ambientCount;
    uint omniCount;
} ubo;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec3 outColor;

vec3  fragNormal;
vec3  fragSpecular;
vec3  fragAlbedo;
vec3  viewDir;
vec3  lightDir;
float attenuation;

void accumulate(const vec3 lightColor)
{
    outColor += (fragAlbedo
              * max(dot(fragNormal, lightDir), 0.0F)
              * lightColor
              + fragSpecular
              * pow(max(dot(fragNormal, normalize(lightDir + viewDir)), 0.0F), 32.0F)
              * lightColor)
              * attenuation;
}

void main()
{
    fragNormal = texture(normalSampler, fragTexCoord).xyz;
    if (fragNormal == vec3(0.0F)) discard;

    outColor = vec3(0.0F);

    const vec3 fragPosition = texture(positionSampler, fragTexCoord).xyz;
               fragNormal   = normalize(fragNormal);
               fragSpecular = texture(specularSampler, fragTexCoord).rgb;
               fragAlbedo   = texture(albedoSampler, fragTexCoord).rgb;
               viewDir      = normalize(ubo.origo - fragPosition);

    attenuation = 1.0F;

    for (uint i = 0; i != ubo.ambientCount; ++i) {
        lightDir = normalize(-ambient.lights[i].direction);

        accumulate(ambient.lights[i].color);
    }

    for (uint i = 0; i != ubo.omniCount; ++i) {
        lightDir = omni.lights[i].position - fragPosition;
        if (lightDir.z <= 0.0F) continue;

        const float distance = length(lightDir);
        if (omni.lights[i].radius <= distance) continue;
        lightDir /= distance;

        attenuation = distance / omni.lights[i].radius;
        attenuation = 1.0F - attenuation * attenuation;

        accumulate(omni.lights[i].color);
    }
}
