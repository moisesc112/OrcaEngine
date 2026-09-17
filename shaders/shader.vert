#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;

	vec3 light_direction;
	vec3 light_color;
	float light_intensity;
	mat4 light_view_projection;

	vec3 camera_position;

	int shadow_enabled;
	int shadow_filter;
	float shadow_constant_bias;
	float shadow_slope_bias;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
	vec4 base_color;
	int use_texture;
} push_constants;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 3) in vec3 in_normal;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_position;
layout(location = 4) out vec4 frag_light_position;

void main() 
{
	mat4 model_matrix = push_constants.model;
	mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));
	vec4 world_position = model_matrix * vec4(in_position, 1.0);

	gl_Position = ubo.proj * ubo.view * model_matrix * vec4(in_position, 1.0);
	frag_color = in_color;
	frag_tex_coord = in_tex_coord;
	frag_normal = normalize(normal_matrix * in_normal);
	frag_position = world_position.xyz;
	frag_light_position = ubo.light_view_projection * world_position;
}