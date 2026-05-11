@tool
class_name DemoPortal
extends MeshInstance3D

@export var enable_copy_properties: bool = false
@export var apply_material: ShaderMaterial
@export var animation_player: AnimationPlayer

@export_tool_button("Open") var open_action = open
@export_tool_button("Close") var close_action = close

var _cummulative_time: float = 0.0
var _shader_uniforms: Array = []
var _stencil_shader: ShaderMaterial

func _ready() -> void:
	apply_material = get_surface_override_material(0).next_pass
	_stencil_shader = get_surface_override_material(0)
	apply_material = _stencil_shader.next_pass
	_shader_uniforms = apply_material.shader.get_shader_uniform_list()
	if apply_material and _stencil_shader and enable_copy_properties:
		for param_dict in _shader_uniforms:
			var param: String = param_dict.name
			if _stencil_shader.get_shader_parameter(param) != apply_material.get_shader_parameter(param):
				_stencil_shader.set_shader_parameter(param, apply_material.get_shader_parameter(param))

func _process(_delta: float) -> void:
	_cummulative_time += _delta
	if _cummulative_time > 2.0 or _stencil_shader == null or apply_material == null:
		_cummulative_time = 0.0
		_stencil_shader = get_surface_override_material(0)
		apply_material = _stencil_shader.next_pass
		_shader_uniforms = apply_material.shader.get_shader_uniform_list()
	
	if apply_material and _stencil_shader and enable_copy_properties:
		for param_dict in _shader_uniforms:
			var param: String = param_dict.name
			if _stencil_shader.get_shader_parameter(param) != apply_material.get_shader_parameter(param):
				_stencil_shader.set_shader_parameter(param, apply_material.get_shader_parameter(param))

func open() -> void:
	if animation_player:
		enable_copy_properties = true
		animation_player.play("open", -1, 0.4)

func close() -> void:
	if animation_player:
		enable_copy_properties = true
		animation_player.play("close", -1, 0.4)
