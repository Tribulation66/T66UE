shader_type spatial;
render_mode unshaded;

group_uniforms texture_settings;
/** texture for the wall */
uniform sampler2D wall_texture : hint_default_white, repeat_enable;
/** texture offset on pixel size (change to fix potential offsets) */
uniform vec2 texture_offset = vec2(0.5, 0.0);
/** texture offset between layers */
uniform vec2 texture_layer_offset = vec2(30.0, 30.0);
/** how should the texture color be combined */
uniform int texture_color_combine: hint_enum("Mult", "Add", "RGBMix", "HSVMix") = 0;


group_uniforms transparency;
/** Should the last layer be transparent? */
uniform bool last_layer_transparent = false;
/** Should the first layer be transparent? */
uniform bool first_layer_transparent = false;
/** Type of transparency transition between layers [Off - no tranparency, Mid|Start|End - specified layer is most opaque] */
uniform int transparency_transition: hint_enum("Off", "Mid", "Start", "End") = 0;
/** Influence the transparency blending curve (1.0 = linear) */
uniform float transparency_falloff: hint_range(0.1, 10.0) = 1.0;
/** add a rim to the portal */
uniform bool rim = true;
/** extra extend of the radius of the rim */
uniform float rim_extra_radius: hint_range(0.0, 10.0) = 0.1;

group_uniforms color;
/** define how many layers are needed to cycle between color_one and color_two, 0 takes all layers */
uniform int color_repeat_layer = 0;
/** layer offset for the color cycle */
uniform int color_offset_layer = 0;
/** first of the blending colors for the hole borders */
uniform vec3 color_one: source_color = vec3(1.0, 1.0, 1.0);
/** second of the blending colors for the hole borders */
uniform vec3 color_two: source_color = vec3(0.0, 0.0, 0.0);
/** influence the color blending curve (1.0 = linear) */
uniform float color_falloff: hint_range(0.1, 10.0) = 1.0;
/** Type of color transition between layers [RGBMix - mix rgb colors, HSVMix - mix HSV colors (hue shift)] */
uniform int color_transition: hint_enum("RGBMix", "HSVMix") = 0;

group_uniforms glow;
/** define how many layers are needed to cycle between full glow and no glow */
uniform int glow_repeat_layer = 0;
/** layer offset for the glow cycle */
uniform int glow_offset_layer = 0;
/** Glow strength */
uniform float glow = 0.0;
/** influence the color blending curve (1.0 = linear) */
uniform float glow_falloff: hint_range(0.1, 10.0) = 1.0;

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

group_uniforms outlines;
/** Make outlines visible */
uniform bool enable_outlines = true;
/** Make thick outlines (doubles) */
uniform bool outlines_double = true;
/** Outline color */
uniform vec4 outline_color: source_color = vec4(0.0, 0.0, 0.0, 1.0);

#include "dither.gdshaderinc"
#include "pixel_portal_base.gdshaderinc"

const vec2 neighbors[8] = vec2[](
	vec2(-1.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, -1.0),
	vec2(0.0, 1.0),
	vec2(-1.0, -1.0),
	vec2(1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0)
);

void fragment() {
	float rotate_width = float(int(rotation_repeat_layer == 0) * layers + rotation_repeat_layer);
	float color_width = float(int(color_repeat_layer == 0) * layers + color_repeat_layer);
	float glow_width = float(int(glow_repeat_layer == 0) * layers + glow_repeat_layer);
	float value = 1.0;
	vec4 texture_color = vec4(1.0);
	float sum_radius_reduction = radius;
	vec2 uv_step = pixel_size / resolution;
	int visible_layer = 0;
	float radius_random_offset_value = float(radius_random_offset);
	vec2 rotated_uv = UV;
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
		visible_layer = i * int(changed) + visible_layer * int(!changed);
		sum_radius_reduction = sum_radius_reduction * (1.0 * float(radius_reduction_type) + (1.0 - radius_reduction) * float(1 - radius_reduction_type));
		sum_radius_reduction -= (1.0 * float(radius_reduction_type)) * radius_reduction;
		rotated_uv = rotated_uv * float(!changed) + circle_uv * float(changed);
	}
	texture_color = texelFetch(wall_texture, ivec2(mod((rotated_uv * resolution) / pixel_size + texture_offset + texture_layer_offset * float(visible_layer), vec2(textureSize(wall_texture, 0)))), 0);
	
	sum_radius_reduction = radius;
	for(int i = 0; i < visible_layer; i++) {
		sum_radius_reduction = sum_radius_reduction * (1.0 * float(radius_reduction_type) + (1.0 - radius_reduction) * float(1 - radius_reduction_type));
		sum_radius_reduction -= (1.0 * float(radius_reduction_type)) * radius_reduction;
	}
	
	int diff_count = 0;
	float rim_value = 0.0;
	if (rim && visible_layer == 0) {
		float random_value = 1.0 - radius_random_offset_value + random(0) * float(layers) * radius_random_offset_value;
		float radius_vary_modifier = (1.0 + radius_variation * sin((TIME + random_value) * radius_speed_variation) * float(vary_radius));
		rim_value = double_dither(rotated_uv, (radius + rim_extra_radius) * radius_vary_modifier, uv_step) * 4.0;
		rim_value = step(2.0, rim_value);
		rim_value = max(rim_value, 0);
	}
	if (enable_outlines && (visible_layer != layers - 1 || value == 1.0) && (rim_value >= 1.0 || visible_layer > 0 || (visible_layer == 0 && !rim))) {
		int double = 1 + int(outlines_double);
		float rotation_value = get_cyclic_value_negative(visible_layer, rotation_offset_layer, rotate_width);
		rotation_value = TIME * rotation * rotation_value;
		float random_value = 1.0 - radius_random_offset_value + random(visible_layer) * float(layers) * radius_random_offset_value;
		float t = float(visible_layer) / float(layers);
		float depth = t * depth_amount;
		vec2 circle_uv = UV + parallax(depth, NORMAL, TANGENT, VIEW);
		float radius_vary_modifier = (1.0 + radius_variation * sin((TIME + float(visible_layer) * radius_variation_offset + random_value) * radius_speed_variation) * float(vary_radius));
		vec2 rotated_circle_uv = rotate_uv(circle_uv, rotation_value);

		for(int n = 0; n < 4 * double; n++) {
			vec2 neighbor_uv = rotated_circle_uv + uv_step * neighbors[n];
			float dither_value = double_dither(neighbor_uv, sum_radius_reduction * radius_vary_modifier, uv_step) * 4.0;
			dither_value = step(2.0, dither_value);
			dither_value = max(dither_value, float(visible_layer)/float(layers));
			diff_count += int(abs(dither_value - value) > 0.0);
		}

		rotation_value = get_cyclic_value_negative(0, rotation_offset_layer, rotate_width);
		rotation_value = TIME * rotation * rotation_value;
		random_value = 1.0 - radius_random_offset_value + random(0) * float(layers) * radius_random_offset_value;
		radius_vary_modifier = (1.0 + radius_variation * sin((TIME + random_value) * radius_speed_variation) * float(vary_radius));
		rotated_circle_uv = rotate_uv(circle_uv, rotation_value);
		for(int n = 0; n < 4 * double; n++) {
			vec2 neighbor_uv = rotated_circle_uv + uv_step * neighbors[n];
			rim_value = double_dither(neighbor_uv, (radius + rim_extra_radius) * radius_vary_modifier, uv_step) * 4.0;
			rim_value = step(2.0, rim_value);
			rim_value = max(rim_value, 0);
			diff_count += int(rim_value < 1.0 || rim_value > 1.0) * int(rim && visible_layer == 0 && first_layer_transparent);
		}
	}
	
	float value_transparent = abs(get_transparent_value(value, float(layers), transparency_transition));
	if (diff_count > 0) {
		ALBEDO = vec3(outline_color.rgb);
		ALPHA = (1.0 - float(transparency_transition > 0) * pow(value_transparent, transparency_falloff)) * outline_color.a;
	} else {
		if ((visible_layer == 0 && first_layer_transparent && (rim_value < 1.0 || !rim)) || (value == 1.0 && last_layer_transparent)) discard;
		float value_color = get_cyclic_value(int(value * float(layers)), color_offset_layer, color_width);
		float value_glow = glow * pow(get_cyclic_value(int(value * float(layers)), glow_offset_layer, glow_width), glow_falloff);
		vec3 color_hsv = get_color_combined(color_one.rgb, color_two.rgb, color_transition, pow(value_color, color_falloff));
		ALBEDO = get_color_combined_texture(texture_color.rgb, color_hsv, texture_color_combine, 0.5) * (1.0 + value_glow);
		ALPHA = 1.0 - float(transparency_transition > 0) * pow(value_transparent, transparency_falloff);
	}
	
}
