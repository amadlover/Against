#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform mat_ubo
{
    mat4 model;
    mat4 view;
    mat4 projection;
} mat_buff;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec4 in_joints;
layout (location = 4) in vec4 in_weights;

layout (location = 0) out vec2 out_uv;

void main ()
{
    gl_Position = mat_buff.projection * mat_buff.view * mat_buff.model * vec4 (in_position, 1);
    out_uv = in_uv;
}