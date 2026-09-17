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

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec3 frag_normal;
layout(location = 3) in vec3 frag_position;
layout(location = 4) in vec4 frag_light_position;

layout(binding = 1) uniform sampler2D tex_sampler;
layout(binding = 2) uniform sampler2D shadow_map;

layout(location = 0) out vec4 out_color;

float CalculateShadowFactor(vec4 light_position, vec3 normal, vec3 light_direction)
{
	vec3 proj_coords = light_position.xyz / light_position.w;
	proj_coords.xy = proj_coords.xy * 0.5 + 0.5;

	if (proj_coords.z > 1.0) {
		return 0.0;
	}

	float current_depth = proj_coords.z;
	float closest_depth = texture(shadow_map, proj_coords.xy).r;

	float ndotl = max(dot(normal, light_direction), 0.0);
	float bias = max(ubo.shadow_slope_bias * (1.0 - ndotl), ubo.shadow_constant_bias);

	float shadow_factor = 0.0;

	if (ubo.shadow_filter == 0) {
		shadow_factor = current_depth - bias > closest_depth ? 1.0 : 0.0;
	}
	else {
		vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
	
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
				shadow_factor += current_depth - bias > pcf_depth ? 1.0 : 0.0;
			}
		}
		shadow_factor /= 9.0;
	}

	return shadow_factor;
}

void main() 
{
	vec3 normal = normalize(frag_normal);
	vec3 light_direction = normalize(-ubo.light_direction);
	vec3 view_direction = normalize(ubo.camera_position - frag_position);
	vec3 reflect_direction = reflect(-light_direction, normal);

	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * ubo.light_color;

	float diffuse_strength = max(dot(normal, light_direction), 0.0);
	vec3 diffuse = diffuse_strength * ubo.light_color;

	float specular_strength = 0.5;
	float shininess = 128.0;

	float specular_amount = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);
	float specular = specular_strength * specular_amount * ubo.light_intensity;

	float shadow_factor = 0.0;

	if (ubo.shadow_enabled != 0) {
		shadow_factor = CalculateShadowFactor(frag_light_position, normal, light_direction);
	}

	vec3 lighting = ambient + (1.0 - shadow_factor) * (diffuse + specular);

	vec3 texture_color = texture(tex_sampler, frag_tex_coord).rgb;

	out_color = vec4(texture_color * lighting, 1.0);
}