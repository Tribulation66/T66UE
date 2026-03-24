shader_type spatial;
render_mode unshaded, depth_draw_opaque, blend_mix;
stencil_mode read, compare_equal, 1;

uniform sampler2D bg_texture;
uniform vec2 speed = vec2(0.02, 0.02);
uniform float scale = 0.00001;
uniform bool invert = false;

void vertex() {

}

void fragment() {
	if (invert) {
		ALBEDO = vec3(1.0) - texture(bg_texture, vec2(UV.x / scale - TIME * speed.x, UV.y / scale + TIME * speed.y)).rgb;
	} else {
		ALBEDO = texture(bg_texture, vec2(UV.x / scale - TIME * speed.x, UV.y / scale + TIME * speed.y)).rgb;
	}
	ALPHA = 1.0;
}

void light() {
	
}