#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 2, binding = 0) uniform sampler2D base_color[32];
layout (set = 2, binding = 1) uniform sampler2D normal[32];
layout (set = 2, binding = 2) uniform sampler2D metalness_roughness[32];
layout (set = 2, binding = 3) uniform sampler2D occlusion[32];
layout (set = 2, binding = 4) uniform sampler2D emission[32];

layout (push_constant) uniform image_index
{
    int index;
} constants;

layout (location = 0) out vec4 out_color;

void main ()
{
    out_color = vec4 (1,1,1,1);
}