#version 460

layout(set = 2, binding = 0) uniform sampler2D albedoSampler;
layout(set = 2, binding = 1) uniform sampler2D specularSampler;
layout(set = 2, binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in mat3 TBN;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec3 fragColor;
layout(location = 5) in vec2 fragTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outSpecular;
layout(location = 3) out vec3 outAlbedo;

void main()
{
    vec4 albedo = texture(albedoSampler, fragTexCoord);
    if (albedo.a <= 0.0) discard;

    outPosition = fragPosition;
    outNormal   = TBN * (texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0);
    outSpecular = fragColor * texture(specularSampler, fragTexCoord).rgb;
    outAlbedo   = fragColor * albedo.rgb;
}
