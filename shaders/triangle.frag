#version 450 core
layout (location = 0) in vec2 texCoord_f;
layout (binding = 0) uniform sampler2D texUnit;
layout (location = 0) out vec4 outFragColor;
void main()
{
    outFragColor = texture(texUnit, texCoord_f);
}