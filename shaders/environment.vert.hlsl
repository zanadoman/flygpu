/*
  FlyGPU
  Copyright (C) 2025 Domán Zana

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

struct UniformBuffer
{
    float4x4 ENV;
};

ConstantBuffer<UniformBuffer> cUniform : register(b0, space1);

static const uint   INDICES[6]   = { 0, 1, 3, 1, 2, 3 };
static const float4 POSITIONS[4] = {
    float4(-1.41421353816986083984375F, 1.41421353816986083984375F, 0.0F, 1.0F),
    float4(-1.41421353816986083984375F, -1.41421353816986083984375F, 0.0F, 1.0F),
    float4(1.41421353816986083984375F, -1.41421353816986083984375F, 0.0F, 1.0F),
    float4(1.41421353816986083984375F, 1.41421353816986083984375F, 0.0F, 1.0F)
};

float4 main(const uint VertexIndex : SV_VertexID) : SV_Position
{
    return mul(POSITIONS[INDICES[VertexIndex]], cUniform.ENV);
}
