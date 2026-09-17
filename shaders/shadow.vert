#version 450

layout(set = 0, binding = 0) uniform ShadowUniformBufferObject {
    mat4 light_view_projection;
} shadow_ubo;

layout(push_constant) uniform ShadowPushConstants {
    mat4 model;
} push_constants;

layout(location = 0) in vec3 in_position;

void main()
{
    gl_Position = shadow_ubo.light_view_projection * push_constants.model * vec4(in_position, 1.0);
}