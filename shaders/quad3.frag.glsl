#version 460

layout(set = 2, binding = 0) uniform sampler2D albedoSampler;
layout(set = 2, binding = 1) uniform sampler2D alphaSampler;
layout(set = 2, binding = 2) uniform sampler2D specularSampler;
layout(set = 2, binding = 3) uniform sampler2D normalSampler;

layout(location = 0) in mat3 TBN;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec3 fragColorTL;
layout(location = 5) in vec3 fragColorBL;
layout(location = 6) in vec3 fragColorBR;
layout(location = 7) in vec3 fragColorTR;
layout(location = 8) in vec2 fragTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outSpecular;
layout(location = 3) out vec3 outAlbedo;

void main()
{
    if (texture(alphaSampler, fragTexCoord).a <= 0.0F) discard;

    const vec2 fragColorCoord = fract(fragTexCoord);
    const vec3 fragColor      = mix(
        mix(fragColorTL, fragColorTR, fragColorCoord.x),
        mix(fragColorBL, fragColorBR, fragColorCoord.x),
        fragColorCoord.y
    );

    outPosition = fragPosition;
    outNormal   = TBN * (texture(normalSampler, fragTexCoord).rgb * 2.0F - 1.0F);
    outSpecular = fragColor * texture(specularSampler, fragTexCoord).rgb;
    outAlbedo   = fragColor * texture(albedoSampler, fragTexCoord).rgb;
}
