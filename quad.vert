#version 460

layout(set = 1, binding = 0) uniform UBO {
    mat4 mvp;
} ubo;

layout(location = 0) out vec4 fragColor;

vec2 positions[4] = vec2[](
    vec2(-0.5,  0.5),
    vec2( 0.5,  0.5),
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5)
);

vec4 colors[4] = vec4[](
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0)
);

uint indices[6] = uint[](0, 1, 2, 2, 1, 3);

void main() {
    gl_Position = ubo.mvp * vec4(positions[indices[gl_VertexIndex]], 0.0, 1.0);
    fragColor   = colors[indices[gl_VertexIndex]];
}
