struct Output
{
    float2 TexCoord       : TEXCOORD0;
    float4 VertexPosition : SV_Position;
};

static const float2 TEXCOORDS[3] = {
    float2(0.0F, 0.0F),
    float2(0.0F, 2.0F),
    float2(2.0F, 0.0F)
};
static const float4 POSITIONS[3] = {
    float4(-1.0F, 1.0F, 0.0F, 1.0F),
    float4(-1.0F, -3.0F, 0.0F, 1.0F),
    float4(3.0F, 1.0F, 0.0F, 1.0F)
};

Output main(const uint VertexIndex : SV_VertexID)
{
    const Output output = { TEXCOORDS[VertexIndex], POSITIONS[VertexIndex] };

    return output;
}
