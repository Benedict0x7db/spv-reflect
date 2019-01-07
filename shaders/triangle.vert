#version 450 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;
layout (location = 0) out vec2 texCoord_f;
layout (binding = 0) uniform MVP
{
 mat4 MVP;
}dd_mvp;

layout (binding = 1) uniform shadow
{
    float shadow;
}dd_shadow;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main()
{
    texCoord_f = texCoord;
    gl_Position = dd_mvp.MVP * position;
}
