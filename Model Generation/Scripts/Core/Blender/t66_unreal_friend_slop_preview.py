"""Shared Blender preview setup for T66 FriendSlop raw Pixal3D assets.

The runtime FriendSlop raw import path uses Unreal's M_GLB_Unlit parent with
neutral Tint/Brightness/Opacity and generated base-color texture binding.  This
module mirrors that behavior in Blender by driving material output from emission
texture color, then adds the locked Hero 1 softbox rig as a review environment.
"""

from __future__ import annotations

import math

import bpy


UNREAL_PREVIEW_RIG_NAME = "T66_UnrealFriendSlopPreview"

SOFTBOX_LIGHTS = (
    {
        "name": "T66_Key_Softbox_L",
        "location": (-5.5, -5.2, 5.0),
        "energy": 470.0,
        "size": 6.5,
    },
    {
        "name": "T66_Key_Softbox_R",
        "location": (5.5, -5.2, 4.6),
        "energy": 360.0,
        "size": 7.0,
    },
    {
        "name": "T66_Top_Soft_Fill",
        "location": (0.0, -1.5, 7.2),
        "energy": 230.0,
        "size": 8.0,
    },
    {
        "name": "T66_Front_Fill",
        "location": (0.0, -7.5, 2.1),
        "energy": 150.0,
        "size": 9.0,
    },
)


def configure_scene_for_unreal_preview(scene: bpy.types.Scene) -> None:
    """Set Blender color/render options to the closest stable Unreal preview."""
    for engine_name in ("BLENDER_EEVEE_NEXT", "BLENDER_EEVEE"):
        try:
            scene.render.engine = engine_name
            break
        except Exception:
            continue

    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0

    if hasattr(scene.eevee, "use_gtao"):
        scene.eevee.use_gtao = False
    if hasattr(scene.eevee, "use_bloom"):
        scene.eevee.use_bloom = False

    if scene.world is None:
        scene.world = bpy.data.worlds.new("T66_UnrealFriendSlopPreviewWorld")
    scene.world.color = (1.0, 1.0, 1.0)
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get("Background")
    if background:
        background.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
        background.inputs[1].default_value = 1.0


def _image_from_upstream_socket(
    socket: bpy.types.NodeSocket,
    visited: set[bpy.types.Node] | None = None,
) -> bpy.types.Image | None:
    if visited is None:
        visited = set()
    for link in socket.links:
        node = link.from_node
        if node in visited:
            continue
        visited.add(node)
        if node.bl_idname == "ShaderNodeTexImage" and getattr(node, "image", None):
            return node.image
        for input_socket in node.inputs:
            image = _image_from_upstream_socket(input_socket, visited)
            if image:
                return image
    return None


def _find_base_color_image(material: bpy.types.Material) -> bpy.types.Image | None:
    if not material or not material.use_nodes or not material.node_tree:
        return None

    for node in material.node_tree.nodes:
        if node.bl_idname != "ShaderNodeBsdfPrincipled":
            continue
        base_color = node.inputs.get("Base Color")
        if not base_color:
            continue
        image = _image_from_upstream_socket(base_color)
        if image:
            return image

    for node in material.node_tree.nodes:
        if node.bl_idname == "ShaderNodeTexImage" and getattr(node, "image", None):
            return node.image
    return None


def convert_material_to_unreal_unlit(
    material: bpy.types.Material,
    *,
    tint: tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    brightness: float = 1.0,
    opacity: float = 1.0,
) -> bool:
    """Mirror M_GLB_Unlit as texture * Tint * Brightness into emission."""
    if not material:
        return False

    base_color_image = _find_base_color_image(material)
    material.use_nodes = True
    material.blend_method = "BLEND" if opacity < 1.0 else "OPAQUE"
    material.use_screen_refraction = False

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    for node in list(nodes):
        nodes.remove(node)

    output = nodes.new(type="ShaderNodeOutputMaterial")
    output.location = (420, 0)

    emission = nodes.new(type="ShaderNodeEmission")
    emission.location = (190, 0)
    emission.inputs["Strength"].default_value = brightness

    if base_color_image:
        texture = nodes.new(type="ShaderNodeTexImage")
        texture.location = (-520, 70)
        texture.image = base_color_image

        tint_node = nodes.new(type="ShaderNodeRGB")
        tint_node.location = (-520, -125)
        tint_node.outputs[0].default_value = tint

        multiply = nodes.new(type="ShaderNodeMix")
        multiply.location = (-145, 30)
        multiply.data_type = "RGBA"
        multiply.factor_mode = "UNIFORM"
        multiply.inputs["Factor"].default_value = 1.0
        multiply.blend_type = "MULTIPLY"

        links.new(texture.outputs["Color"], multiply.inputs["A"])
        links.new(tint_node.outputs["Color"], multiply.inputs["B"])
        links.new(multiply.outputs["Result"], emission.inputs["Color"])
    else:
        emission.inputs["Color"].default_value = tint

    links.new(emission.outputs["Emission"], output.inputs["Surface"])

    material["T66_UnrealPreview"] = "M_GLB_Unlit"
    material["T66_UnrealPreview_Tint"] = tint
    material["T66_UnrealPreview_Brightness"] = brightness
    material["T66_UnrealPreview_Opacity"] = opacity
    if base_color_image:
        material["T66_UnrealPreview_Image"] = base_color_image.name
    return True


def convert_scene_materials_to_unreal_unlit(scene: bpy.types.Scene) -> int:
    converted = 0
    seen: set[str] = set()
    for obj in scene.objects:
        if obj.type != "MESH":
            continue
        for slot in obj.material_slots:
            material = slot.material
            if not material or material.name in seen:
                continue
            if convert_material_to_unreal_unlit(material):
                converted += 1
                seen.add(material.name)
    return converted


def remove_existing_preview_rig(scene: bpy.types.Scene) -> None:
    names = {entry["name"] for entry in SOFTBOX_LIGHTS}
    for obj in list(scene.objects):
        if obj.name in names or obj.name == UNREAL_PREVIEW_RIG_NAME:
            bpy.data.objects.remove(obj, do_unlink=True)


def add_friend_slop_softbox_rig(scene: bpy.types.Scene) -> None:
    """Add the locked Hero 1 softbox rig as secondary review lighting."""
    remove_existing_preview_rig(scene)
    rig = bpy.data.objects.new(UNREAL_PREVIEW_RIG_NAME, None)
    scene.collection.objects.link(rig)

    for entry in SOFTBOX_LIGHTS:
        data = bpy.data.lights.new(name=entry["name"], type="AREA")
        data.energy = entry["energy"]
        data.size = entry["size"]
        data.use_shadow = False
        obj = bpy.data.objects.new(entry["name"], data)
        obj.location = entry["location"]
        obj.parent = rig
        direction = mathutils_direction_to_origin(obj.location)
        obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
        scene.collection.objects.link(obj)

    rig["T66_Source"] = "Hero_1_Chad_Male_Rubber_20260605 locked softbox rig"
    rig["T66_Role"] = "Secondary review environment; materials remain Unreal unlit"


def mathutils_direction_to_origin(location: tuple[float, float, float]):
    from mathutils import Vector

    loc = Vector(location)
    return Vector((0.0, 0.0, 1.8)) - loc


def apply_unreal_friend_slop_preview(scene: bpy.types.Scene) -> dict[str, object]:
    configure_scene_for_unreal_preview(scene)
    material_count = convert_scene_materials_to_unreal_unlit(scene)
    add_friend_slop_softbox_rig(scene)
    scene["T66_UnrealPreview_Profile"] = (
        "FriendSlop M_GLB_Unlit: texture * Tint(1,1,1,1) * Brightness(1), "
        "fixed Standard color management, Hero 1 softbox review rig."
    )
    return {
        "profile": "T66 Unreal FriendSlop unlit preview",
        "materials_converted": material_count,
        "softbox_lights": [entry["name"] for entry in SOFTBOX_LIGHTS],
    }
