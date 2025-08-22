#version 460 core

layout(location = 0) out vec2 fragTexCoord;

const vec4 positions[3] = vec4[](
    vec4(-1.0F, 1.0F, 0.0F, 1.0F),
    vec4(-1.0F, -3.0F, 0.0F, 1.0F),
    vec4(3.0F, 1.0F, 0.0F, 1.0F)
);

const vec2 fragTexCoords[3] = vec2[](
    vec2(0.0F, 0.0F),
    vec2(0.0F, 2.0F),
    vec2(2.0F, 0.0F)
);

void main()
{
    gl_Position  = positions[gl_VertexIndex];
    fragTexCoord = fragTexCoords[gl_VertexIndex];
}
