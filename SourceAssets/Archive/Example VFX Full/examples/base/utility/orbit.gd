extends Node3D

signal rotation_swap
signal rotation_close_to_swap

@export var mouse_sensitivity: float = 0.01
@export var zoom_speed: float = 0.5
@export var auto_rotate: bool = false

var _rot_x: float = 0.0
var _rot_y: float = 0.0
var _cummulative_time: float = 0.0
var _rotating_back: bool = false

@onready var cam: Camera3D = $Camera3D

func _ready():
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
	
func _process(delta: float) -> void:
	if not auto_rotate:
		return
		
	var _old_rotation_degrees:  float = rotation_degrees.y
	_cummulative_time += delta
	rotation_degrees.y = sign(sin(_cummulative_time)) * (1.0 - pow(abs(sin(_cummulative_time)), 0.5)) * 70.0
	if sign(_old_rotation_degrees) != sign(rotation_degrees.y):
		rotation_swap.emit()
		_rotating_back = false
	if sign(rotation_degrees.y) > 0.0 && rotation_degrees.y - _old_rotation_degrees > 0.0 && abs(rotation_degrees.y) < 20.0 && abs(rotation_degrees.y) > 5.0:
		if not _rotating_back:
			rotation_close_to_swap.emit()
			_rotating_back = true
	elif sign(rotation_degrees.y) < 0.0 && rotation_degrees.y - _old_rotation_degrees < 0.0 && abs(rotation_degrees.y) < 20.0 && abs(rotation_degrees.y) > 5.0:
		if not _rotating_back:
			rotation_close_to_swap.emit()
			_rotating_back = true

func _input(event):
	if event.is_action_pressed("ui_cancel"):
		Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
	
	if event.is_action_pressed("ui_accept"):
		Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
		
	if event is InputEventMouseMotion:
		_rot_x -= event.relative.y * mouse_sensitivity
		_rot_y -= event.relative.x * mouse_sensitivity
		_rot_x = clamp(_rot_x, -80, 80)
		if not auto_rotate:
			rotation_degrees.x = _rot_x
			rotation_degrees.y = _rot_y

	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_WHEEL_UP:
			cam.translate(Vector3(0, 0, -zoom_speed))
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			cam.translate(Vector3(0, 0, zoom_speed))
