#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;

	vec3 light_direction;
	vec3 light_color;
	vec3 light_intensity;
} ubo;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec3 frag_normal;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main() {
	out_color = texture(tex_sampler, frag_tex_coord);
}