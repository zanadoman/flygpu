#version 460 core

layout(set = 2, binding = 0) uniform sampler2D albedoSampler;
layout(set = 2, binding = 1) uniform sampler2D alphaSampler;
layout(set = 2, binding = 2) uniform sampler2D specularSampler;
layout(set = 2, binding = 3) uniform sampler2D normalSampler;

layout(location = 0) in struct
{
    mat3 TBN;
    vec3 position;
    vec3 colorTL;
    vec3 colorBL;
    vec3 colorBR;
    vec3 colorTR;
    vec2 texCoord;
} frag;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outSpecular;
layout(location = 3) out vec3 outAlbedo;

void main()
{
    if (texture(alphaSampler, frag.texCoord).a <= 0.0F) discard;

    const vec2 fragColorCoord = fract(frag.texCoord);
    const vec3 fragColor      = mix(
        mix(frag.colorTL, frag.colorTR, fragColorCoord.x),
        mix(frag.colorBL, frag.colorBR, fragColorCoord.x),
        fragColorCoord.y
    );

    outPosition = frag.position;
    outNormal   = frag.TBN * (texture(normalSampler, frag.texCoord).rgb * 2.0F - 1.0F);
    outSpecular = fragColor * texture(specularSampler, frag.texCoord).rgb;
    outAlbedo   = fragColor * texture(albedoSampler, frag.texCoord).rgb;
}
