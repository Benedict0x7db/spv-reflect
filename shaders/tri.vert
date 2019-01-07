#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 dirDis;
layout (binding = 0) uniform MVP
{
    mat4 MVP;
}gl_MVP;
layout (binding = 1) uniform half_width
{
    float half_width;
}gl_half_width;
layout (binding = 2) uniform halfPixelWidth_rev
{
    float halfPixelWidth_rev;
}gl_halfPixelWidth_rev;
layout (binding = 3) uniform unit_max
{
    float unit_max;
}gl_unit_max;
layout (binding = 4) uniform h_w
{
    float h_w;
}gl_h_w;
layout (binding = 5) uniform offset
{
    float offset;
}gl_offset;
layout (location = 0) out vec3  umf_df_adf;
void main()
{
    vec4 pos0 = gl_MVP.MVP * vec4(position.xy, 0.0, 1.0);
    vec4 pos1 = gl_MVP.MVP * vec4(position.xy + dirDis.xy * gl_half_width.half_width , 0.0, 1.0);
    umf_df_adf.z = position.z + dirDis.z * gl_offset.offset;
    gl_Position = pos1;
    if (dirDis.z == 0.0)
        gl_Position = pos0;
    vec2 p0 = pos0.xy / pos0.w;
    vec2 p1 = pos1.xy / pos1.w;
    p1 -= p0;
    p1.y *= gl_h_w.h_w;
    float ratio = length(p1) * gl_halfPixelWidth_rev.halfPixelWidth_rev;
    ratio = min(ratio, 1.3);
    umf_df_adf.x = gl_unit_max.unit_max * ratio;
    umf_df_adf.y = dirDis.z * umf_df_adf.x;
}
