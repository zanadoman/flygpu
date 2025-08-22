#version 460 core

layout(location = 0)  in mat4 MODEL;
layout(location = 4)  in mat4 MVP;
layout(location = 8)  in mat3 TBN;
layout(location = 11) in vec3 inColorTL;
layout(location = 12) in vec3 inColorBL;
layout(location = 13) in vec3 inColorBR;
layout(location = 14) in vec3 inColorTR;
layout(location = 15) in vec4 inTexCoord;

layout(location = 0) out struct
{
    mat3 TBN;
    vec3 position;
    vec3 colorTL;
    vec3 colorBL;
    vec3 colorBR;
    vec3 colorTR;
    vec2 texCoord;
} frag;

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

    gl_Position   = MVP * positions[i];
    frag.TBN      = TBN;
    frag.position = (MODEL * positions[i]).xyz;
    frag.colorTL  = inColorTL;
    frag.colorBL  = inColorBL;
    frag.colorBR  = inColorBR;
    frag.colorTR  = inColorTR;
    frag.texCoord = vec2[](
        inTexCoord.xy,
        vec2(inTexCoord.x, inTexCoord.w),
        inTexCoord.zw,
        vec2(inTexCoord.z, inTexCoord.y)
    )[i];
}
