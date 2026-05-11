shader_type spatial;
render_mode unshaded;
stencil_mode write, compare_always, 1;

group_uniforms transparency;
/** Should the last layer be transparent? */
uniform bool last_layer_transparent = false;
/** Should the first layer be transparent? */
uniform bool first_layer_transparent = false;
/** Type of transparency transition between layers [Off - no tranparency, Mid|Start|End - specified layer is most opaque] */
uniform int transparency_transition: hint_enum("Off", "Mid", "Start", "End") = 0;
/** Influence the transparency blending curve (1.0 = linear) */
uniform float transparency_falloff: hint_range(0.1, 10.0) = 1.0;

group_uniforms hole;
/** Defines the maximum depth of the parallax layers */
uniform float depth_amount = 1.0;
/** Number of layers */
uniform int layers = 12;
/** Base radius of the holes */
uniform float radius: hint_range(0.0, 10.0) = 0.3;
/** Value for radius reduction calculations (higher values make the holes on deeper levels smaller faster) */
uniform float radius_reduction = 0.1;
/** Radius reduction calculation method */
uniform int radius_reduction_type: hint_enum("Exponential", "linear") = 0;
/** Cyclic additional variation of the radius between layers based on TIME */
uniform bool vary_radius = true;
/** Proportional variation amount (percentage of the layers radius) */
uniform float radius_variation = 0.1;
/** Speed of the variation change */
uniform float radius_speed_variation = 1.0;
/** Variation offset between layers */
uniform float radius_variation_offset = 1.0;
/** Randomize an additional offset between layers (to avoid a wave pattern) */
uniform bool radius_random_offset = true;
/** Center of the circular effect in UV coordinates */
uniform vec2 center = vec2(0.5, 0.5);
/** define how many layers are needed to cycle between rotation values, 0 takes all layers */
uniform int rotation_repeat_layer = 0;
/** layer offset for the rotation cycle */
uniform int rotation_offset_layer = 0;
/** Rotation strength */
uniform float rotation = 0.0;


group_uniforms dither;
/** Size of the pixel blocks used for quantization */
uniform float pixel_size : hint_range(0.0, 100.0) = 10.0;
/** Resolution of the viewport/texture */
uniform vec2 resolution = vec2(512.0, 512.0);
/** Offset applied to the dither pattern lookup */
uniform ivec2 dither_offset = ivec2(2, 0);
/** Size of the dither matrix to use (2x2, 4x4, or 8x8) */
uniform int bayer_size : hint_enum("2x2", "4x4", "8x8") = 2;
/** Whether to interpolate the radius calculation for smoother edges */
uniform bool interpolate = true;
/** Controls how quickly the dither pattern density changes (blend border size) */
uniform float falloff: hint_range(0.1, 10.0) = 2.5;

#include "dither.gdshaderinc"
#include "pixel_portal_base.gdshaderinc"


void fragment() {
	float rotate_width = float(int(rotation_repeat_layer == 0) * layers + rotation_repeat_layer);
	float value = 1.0;
	float sum_radius_reduction = radius;
	vec2 uv_step = pixel_size / resolution;
	int visible_layer = 0;
	float radius_random_offset_value = float(radius_random_offset);
	for(int i = 0; i < layers; i++) {
		float random_value = 1.0 - radius_random_offset_value + random(i) * float(layers) * radius_random_offset_value;
		float t = float(i) / float(layers);
		float depth = t * depth_amount;
		vec2 circle_uv = UV + parallax(depth, NORMAL, TANGENT, VIEW);
		float rotation_value = get_cyclic_value_negative(i, rotation_offset_layer, rotate_width);
		rotation_value = TIME * rotation * rotation_value;
		circle_uv = rotate_uv(circle_uv, rotation_value);
		float radius_vary_modifier = (1.0 + radius_variation * sin((TIME + float(i) * radius_variation_offset + random_value) * radius_speed_variation) * float(vary_radius));
		float dither_value = double_dither(circle_uv, sum_radius_reduction * radius_vary_modifier, uv_step) * 4.0;
		dither_value = step(2.0, dither_value);
		dither_value = max(dither_value, float(i)/float(layers));
		value = min(value, dither_value);
		bool changed = abs(value - dither_value) < 0.01;
		visible_layer = i * int(changed) + visible_layer * int(!changed);		sum_radius_reduction = sum_radius_reduction * (1.0 * float(radius_reduction_type) + (1.0 - radius_reduction) * float(1 - radius_reduction_type));
		sum_radius_reduction -= (1.0 * float(radius_reduction_type)) * radius_reduction;
	}
	value = get_transparent_value(value, float(layers), transparency_transition);
	if ((visible_layer == 0 && first_layer_transparent) || value < 0.0) discard;
	ALBEDO = vec3(1.0);
	ALPHA = 1.0;
}
