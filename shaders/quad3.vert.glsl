#version 460 core

layout(location = 0)  in mat4 inMODEL;
layout(location = 4)  in mat4 inMVP;
layout(location = 8)  in mat3 inTBN;
layout(location = 11) in vec3 inColorTL;
layout(location = 12) in vec3 inColorBL;
layout(location = 13) in vec3 inColorBR;
layout(location = 14) in vec3 inColorTR;
layout(location = 15) in vec4 inTexCoord;

layout(location = 0) out mat3 fragTBN;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec3 fragColorTL;
layout(location = 5) out vec3 fragColorBL;
layout(location = 6) out vec3 fragColorBR;
layout(location = 7) out vec3 fragColorTR;
layout(location = 8) out vec2 fragTexCoord;

const uint indices[6] = uint[](0, 1, 3, 1, 2, 3);

const vec4 positions[4] = vec4[](
    vec4(-0.5F, 0.5F, 0.0F, 1.0F),
    vec4(-0.5F, -0.5F, 0.0F, 1.0F),
    vec4(0.5F, -0.5F, 0.0F, 1.0F),
    vec4(0.5F, 0.5F, 0.0F, 1.0F)
);

void main()
{
    const uint i = indices[gl_VertexIndex];

    gl_Position  = inMVP * positions[i];
    fragTBN      = inTBN;
    fragPosition = (inMODEL * positions[i]).xyz;
    fragColorTL  = inColorTL;
    fragColorBL  = inColorBL;
    fragColorBR  = inColorBR;
    fragColorTR  = inColorTR;
    switch (i) {
    case 0: fragTexCoord = inTexCoord.xy; break;
    case 1: fragTexCoord = inTexCoord.xw; break;
    case 2: fragTexCoord = inTexCoord.zw; break;
    case 3: fragTexCoord = inTexCoord.zy; break;
    }
}
