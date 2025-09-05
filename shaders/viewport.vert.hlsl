struct VSOutput
{
    float4 VertexPosition : SV_Position;
    float2 TexCoord       : TEXCOORD0;
};

static const float4 POSITIONS[3] = {
    float4(-1.0F, 1.0F, 0.0F, 1.0F),
    float4(-1.0F, -3.0F, 0.0F, 1.0F),
    float4(3.0F, 1.0F, 0.0F, 1.0F)
};

static const float2 TEXCOORDS[3] = {
    float2(0.0F, 0.0F),
    float2(0.0F, 2.0F),
    float2(2.0F, 0.0F)
};

[shader("vertex")]
VSOutput vertMain(const uint VertexIndex : SV_VertexID)
{
    VSOutput output;

    output.VertexPosition = POSITIONS[VertexIndex];
    output.TexCoord       = TEXCOORDS[VertexIndex];

    return output;
}
