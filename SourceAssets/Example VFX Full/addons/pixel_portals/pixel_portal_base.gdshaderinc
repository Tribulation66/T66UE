#include "color.gdshaderinc"

float random(int x)
{
    return fract(sin(float(x)) * 521695.2921843);
}

vec2 parallax(float l_depth, vec3 l_normal, vec3 l_tangent, vec3 l_view) {
	vec3 normal = normalize(l_normal);
	vec3 tangent = normalize(l_tangent);
	vec3 bitangent = cross(normal, tangent);
	vec3 view = normalize(l_view);
	vec3 view_tangent = vec3(dot(view, tangent), dot(view, bitangent), dot(view, normal));
	vec2 offset = (view_tangent.xy / max(view_tangent.z, 0.001)) * l_depth;
	offset = vec2(-offset.x, offset.y);
	return offset;
}

float get_transparent_value(float l_value, float l_layers, int l_transition) {
	float start_value = l_value;
	float end_value = -(1.0 - l_value);
	float layer_value = l_value * l_layers;
	float mid_value = (layer_value - l_layers / 2.0) / (0.5 * l_layers);
	return mid_value * float(l_transition == 1) + start_value * float(l_transition == 2) + end_value * float(l_transition == 3);
}

vec3 get_color_combined(vec3 l_col_a, vec3 l_col_b, int l_combine_type, float l_value) {
	if (l_combine_type == 0) {
		return mix(l_col_a, l_col_b, l_value);
	} else {
		return hsv2rgb(mix(rgb2hsv(l_col_a.rgb), rgb2hsv(l_col_b.rgb), l_value));
	}
}

vec3 get_color_combined_texture(vec3 l_col_a, vec3 l_col_b, int l_combine_type, float l_value) {
	vec3 hsv_a = rgb2hsv(l_col_a.rgb);
	vec3 hsv_b = rgb2hsv(l_col_b.rgb);
	vec3 hsv_combined = vec3(hsv_a.r + hsv_b.r, hsv_a.g, hsv_a.b);
	return l_col_a * l_col_b * float(l_combine_type == 0) +
		(l_col_a + l_col_b) * float(l_combine_type == 1) +
		mix(l_col_a, l_col_b, l_value) * float(l_combine_type == 2) +
		hsv2rgb(hsv_combined) * float(l_combine_type == 3);
}

vec2 rotate_uv(vec2 l_uv, float l_rotation) {
	return vec2(
      cos(l_rotation) * (l_uv.x - 0.5) + sin(l_rotation) * (l_uv.y - 0.5) + 0.5,
      cos(l_rotation) * (l_uv.y - 0.5) - sin(l_rotation) * (l_uv.x - 0.5) + 0.5
    );
}

float get_cyclic_value(int layer, int offset, float width) {
	if (width == 0.0) return 0.0;
	float value = float(layer) + float(offset);
	return abs(mod(value, 2.0 * width) - width) / width;
}

float get_cyclic_value_negative(int layer, int offset, float width) {
	if (width == 0.0) return 0.0;
	float value = float(layer) + float(offset);
	return (mod(value, 2.0 * width) - width) / width;
}