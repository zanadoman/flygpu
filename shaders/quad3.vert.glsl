#version 460

layout(location = 0)  in vec4 MODEL0;
layout(location = 1)  in vec4 MODEL1;
layout(location = 2)  in vec4 MODEL2;
layout(location = 3)  in vec4 MODEL3;
layout(location = 4)  in vec4 MVP0;
layout(location = 5)  in vec4 MVP1;
layout(location = 6)  in vec4 MVP2;
layout(location = 7)  in vec4 MVP3;
layout(location = 8)  in vec3 TBN0;
layout(location = 9)  in vec3 TBN1;
layout(location = 10) in vec3 TBN2;
layout(location = 11) in vec3 inColorTL;
layout(location = 12) in vec3 inColorBL;
layout(location = 13) in vec3 inColorBR;
layout(location = 14) in vec3 inColorTR;
layout(location = 15) in vec4 inTexCoord;

layout(location = 0) out mat3 TBN;
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

    gl_Position  = mat4(MVP0, MVP1, MVP2, MVP3) * positions[i];
    TBN          = mat3(TBN0, TBN1, TBN2);
    fragPosition = (mat4(MODEL0, MODEL1, MODEL2, MODEL3) * positions[i]).xyz;
    fragColorTL  = inColorTL;
    fragColorBL  = inColorBL;
    fragColorBR  = inColorBR;
    fragColorTR  = inColorTR;
    fragTexCoord = vec2[](
        inTexCoord.xy,
        vec2(inTexCoord.x, inTexCoord.w),
        inTexCoord.zw,
        vec2(inTexCoord.z, inTexCoord.y)
    )[i];
}
