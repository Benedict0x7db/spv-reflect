#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout (location = 0) in vec2 texCoord_f;
layout (location = 1) in vec3 eye_normal;
layout (binding = 0)  uniform sampler2D texUnit;
layout (location = 0) out vec4 outFragColor;
void main()
{
    vec3 lightDir = vec3(0.0, 0.0, 1.0);
    vec3 normal = normalize(eye_normal);
    float intensity = max(dot(lightDir, normal), 0.0) * 0.80;
    outFragColor = (0.15 + intensity) * texture(texUnit, texCoord_f);
}
