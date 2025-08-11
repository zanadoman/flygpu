#version 460

layout(location = 0) out vec2 fragTexCoord;

const uint indices[6] = uint[](0, 1, 3, 1, 2, 3);

const vec4 positions[4] = vec4[](
    vec4(-1.0F, 1.0F, 0.0F, 1.0F),
    vec4(-1.0F, -1.0F, 0.0F, 1.0F),
    vec4(1.0F, -1.0F, 0.0F, 1.0F),
    vec4(1.0F, 1.0F, 0.0F, 1.0F)
);

const vec2 fragTexCoords[4] = vec2[](
    vec2(0.0F, 0.0F),
    vec2(0.0F, 1.0F),
    vec2(1.0F, 1.0F),
    vec2(1.0F, 0.0F)
);

void main()
{
    const uint i = indices[gl_VertexIndex];

    gl_Position  = positions[i];
    fragTexCoord = fragTexCoords[i];
}
