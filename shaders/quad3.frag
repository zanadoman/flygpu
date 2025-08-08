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

#version 460

layout(set = 2, binding = 0) uniform sampler2D albedoSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;

layout(location = 0) in mat3 TBN;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec4 fragColor;
layout(location = 5) in vec2 fragTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outDiffuse;

void main()
{
    vec4 albedo = texture(albedoSampler, fragTexCoord);
    if (albedo.a <= 0.0) discard;
    outPosition = fragPosition;
    outNormal   = normalize(TBN * (texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0));
    outDiffuse  = vec4((fragColor * albedo).rgb, 0.2);
}
