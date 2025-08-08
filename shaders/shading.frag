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

layout(set = 2, binding = 0) uniform sampler2D positionSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;
layout(set = 2, binding = 2) uniform sampler2D diffuseSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec3 outColor;

const vec3 light = normalize(vec3(1.0 / 3.0, 1.0 / 3.0, 1.0));

void main()
{
    vec4 diffuse = texture(diffuseSampler, fragTexCoord);
    if (diffuse.a <= 0.0) discard;
    outColor = diffuse.rgb * max(dot(texture(normalSampler, fragTexCoord).xyz, light), 0.0);
}
