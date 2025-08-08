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

const float shininess = 64.0;

const float constant  = 1.0;
const float linear    = 0.09;
const float quadratic = 0.032;

const vec3 ambient       = vec3(0.1);
const vec3 lightPosition = vec3(0.0, 0.0, 0.0);
const vec3 lightColor    = vec3(1.0);

void main()
{
    vec3 normal = texture(normalSampler, fragTexCoord).xyz;
    if (normal == vec3(0.0)) discard;

    vec3  position = texture(positionSampler, fragTexCoord).xyz;
    vec3  albedo   = texture(diffuseSampler, fragTexCoord).rgb;
    float specular = texture(diffuseSampler, fragTexCoord).a;

    vec3  lightDir = lightPosition - position;
    if (lightDir == vec3(0.0)) discard;
    float distance = length(lightDir);
    lightDir       = normalize(lightDir);

    float NdotL   = max(dot(normal, lightDir), 0.0);
    vec3  diffuse = albedo * NdotL;

    vec3  viewDir = normalize(-position);
    vec3  halfDir = normalize(lightDir + viewDir);
    float NdotH   = max(dot(normal, halfDir), 0.0);
    specular      = pow(NdotH, shininess) * specular;

    outColor = ambient * albedo + lightColor * (diffuse + vec3(specular))
             * (1.0 / (constant + linear * distance + quadratic * distance * distance));
}
