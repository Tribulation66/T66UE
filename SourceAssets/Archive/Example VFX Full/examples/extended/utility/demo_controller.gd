extends Node3D

@export var portals: Array[DemoPortal] = []
var _active_portal: int = 0
@onready var node_3d: Node3D = $Node3D

func _ready() -> void:
	node_3d.rotation_swap.connect(_switch_portal)
	node_3d.rotation_close_to_swap.connect(_close_portal)
	for portal in portals:
		portal.apply_material.set_shader_parameter("radius", 0.0)

func _input(event: InputEvent) -> void:
	if event.is_action_released("ui_accept"):
		_switch_portal()
	
	if event.is_action_released("ui_up"):
		portals[_active_portal].open()
		
	if event.is_action_released("ui_down"):
		portals[_active_portal].close()
		

func _switch_portal() -> void:
	if _active_portal < portals.size():
		portals[_active_portal].visible = false
	_active_portal += 1
	if _active_portal >= portals.size():
		_active_portal = 0
	if _active_portal < portals.size():
		portals[_active_portal].visible = true
	portals[_active_portal].open()
	
func _close_portal() -> void:
	portals[_active_portal].close()
	
