#!/usr/bin/env python3
"""
Build reusable skeletal FBX sources for the default animated ToonStyle heroes.

Run with Blender in background mode:

  blender --background --python "Model Generation/Rigging and Animation/Tools/create_animated_toonstyle_hero_sources.py"

The script uses the accepted Hero_1_Chad Rigify/Quaternius scene as a template:
- export Hero_1_Chad from the accepted rigged mesh directly,
- import other Pixal3D hero GLBs so their original UVs/material layout are preserved,
- rotate and scale them into the template pose,
- transfer template deformation weights by nearest surface,
- attach the mesh to the template Rigify armature,
- export one skeletal mesh FBX plus Idle/Walk/Jump/Roll animation FBXs.

The Unreal import step consumes the generated manifest.
"""

from __future__ import annotations

import json
import math
import os
import shutil
from pathlib import Path

import bpy
from mathutils import Vector


PROJECT_ROOT = Path(os.environ.get("T66_PROJECT_ROOT", r"C:\UE\T66"))
SOURCE_ROOT = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_SOURCE_ROOT",
        PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "HeroDemoLineup_20260522_AccuRig" / "Outputs",
    )
)
TEMPLATE_BLEND = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_TEMPLATE_BLEND",
        PROJECT_ROOT
        / "Model Generation"
        / "Runs"
        / "Pixal3D"
        / "HeroDemoLineup_20260522_AccuRig"
        / "RigifyWalkProbe_20260522"
        / "Hero_1_Chad_Rigify_AnimatedToonStyle_Template.blend",
    )
)
OUTPUT_ROOT = Path(
    os.environ.get(
        "T66_ANIMATED_TOONSTYLE_OUTPUT_ROOT",
        PROJECT_ROOT / "Model Generation" / "Rigging and Animation" / "Runs" / "AnimatedToonStyleHeroes_20260522",
    )
)

DEFAULT_HEROES = [
    "Hero_1_Chad",
    "Hero_1_Stacy",
    "Hero_2_Chad",
    "Hero_2_Stacy",
    "Hero_3_Chad",
    "Hero_3_Stacy",
    "Hero_4_Chad",
    "Hero_4_Stacy",
    "Hero_5_Chad",
    "Hero_5_Stacy",
]

HUMANOID_GUIDELINE_RECORDS = [
    {
        "visual_id": "Hero_1_Chad_DemoSkin",
        "source_id": "Hero_1_Chad_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_1/Chad/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_1_Stacy_DemoSkin",
        "source_id": "Hero_1_Stacy_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_1/Stacy/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_2_Chad_DemoSkin",
        "source_id": "Hero_2_Chad_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_2/Chad/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_2_Stacy_DemoSkin",
        "source_id": "Hero_2_Stacy_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_2/Stacy/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_3_Chad_DemoSkin",
        "source_id": "Hero_3_Chad_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_3/Chad/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_3_Stacy_DemoSkin",
        "source_id": "Hero_3_Stacy_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_3/Stacy/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_4_Chad_DemoSkin",
        "source_id": "Hero_4_Chad_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_4/Chad/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_4_Stacy_DemoSkin",
        "source_id": "Hero_4_Stacy_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_4/Stacy/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_5_Chad_DemoSkin",
        "source_id": "Hero_5_Chad_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_5/Chad/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Hero_5_Stacy_DemoSkin",
        "source_id": "Hero_5_Stacy_Demo",
        "target_dir": "/Game/Characters/Heroes/Hero_5/Stacy/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    },
    {
        "visual_id": "Companion_01",
        "source_id": "Companion_RapVixenLightskinBlack_Regular",
        "target_dir": "/Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_01_DemoSkin",
        "source_id": "Companion_RapVixenLightskinBlack_Demo",
        "target_dir": "/Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_02",
        "source_id": "Companion_BarMaidenBlonde_Regular",
        "target_dir": "/Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_02_DemoSkin",
        "source_id": "Companion_BarMaidenBlonde_Demo",
        "target_dir": "/Game/Characters/Companions/Companion_02/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_03",
        "source_id": "Companion_CollegeBrunette_Regular",
        "target_dir": "/Game/Characters/Companions/Companion_03/Default/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_03_DemoSkin",
        "source_id": "Companion_CollegeBrunette_Demo",
        "target_dir": "/Game/Characters/Companions/Companion_03/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_04",
        "source_id": "Companion_OfficeLadyBlackhair_Regular",
        "target_dir": "/Game/Characters/Companions/Companion_04/Default/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
    {
        "visual_id": "Companion_04_DemoSkin",
        "source_id": "Companion_OfficeLadyBlackhair_Demo",
        "target_dir": "/Game/Characters/Companions/Companion_04/DemoSkin/AnimatedToonStyle",
        "mesh_relative_scale": "(X=0.588235,Y=0.588235,Z=0.588235)",
        "asset_kind": "companion",
    },
]

REQUESTED_HEROES = [
    hero_id.strip()
    for hero_id in os.environ.get("T66_ANIMATED_TOONSTYLE_HERO_IDS", "").split(",")
    if hero_id.strip()
]

REQUESTED_RECORDS_JSON = os.environ.get("T66_ANIMATED_TOONSTYLE_RECORDS_JSON", "").strip()
SOURCE_PRESET = os.environ.get("T66_ANIMATED_TOONSTYLE_SOURCE_PRESET", "default_heroes").strip()
DEFAULT_SOURCE_YAW_DEGREES = float(os.environ.get("T66_ANIMATED_TOONSTYLE_SOURCE_YAW_DEGREES", "180.0"))

ACTION_SOURCES = {
    "Idle": "Idle_No_Loop",
    "Walk": "Walk_Fwd_Loop_LegsTorsoOnly",
    "Jump": "DoubleJump_LegsTorsoOnly",
    "Roll": "Roll_LegsTorsoOnly",
}

TEMPLATE_RIG_NAME = "Hero_1_Chad_Rigify_Rig"
TEMPLATE_MESH_NAME = "Hero_1_Chad_RigifyProbe_Mesh"
EXACT_TEMPLATE_HERO_IDS = {"Hero_1_Chad"}

FK_POSE_BONES = [
    "root",
    "torso",
    "hips",
    "chest",
    "neck",
    "head",
    "thigh_parent.L",
    "thigh_fk.L",
    "shin_fk.L",
    "foot_fk.L",
    "toe_fk.L",
    "thigh_ik.L",
    "foot_ik.L",
    "foot_spin_ik.L",
    "foot_heel_ik.L",
    "toe_ik.L",
    "thigh_parent.R",
    "thigh_fk.R",
    "shin_fk.R",
    "foot_fk.R",
    "toe_fk.R",
    "thigh_ik.R",
    "foot_ik.R",
    "foot_spin_ik.R",
    "foot_heel_ik.R",
    "toe_ik.R",
]


def world_bbox(obj: bpy.types.Object) -> tuple[Vector, Vector]:
    if obj.type == "MESH" and obj.data and obj.data.vertices:
        coords = [obj.matrix_world @ vertex.co for vertex in obj.data.vertices]
    else:
        coords = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    min_v = Vector((min(v.x for v in coords), min(v.y for v in coords), min(v.z for v in coords)))
    max_v = Vector((max(v.x for v in coords), max(v.y for v in coords), max(v.z for v in coords)))
    return min_v, max_v


def center_xy(min_v: Vector, max_v: Vector) -> Vector:
    return Vector(((min_v.x + max_v.x) * 0.5, (min_v.y + max_v.y) * 0.5, min_v.z))


def select_only(objects: list[bpy.types.Object]) -> None:
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    if objects:
        bpy.context.view_layer.objects.active = objects[0]


def apply_object_transform(obj: bpy.types.Object, location: bool, rotation: bool, scale: bool) -> None:
    select_only([obj])
    bpy.ops.object.transform_apply(location=location, rotation=rotation, scale=scale)


def imported_meshes(before_names: set[str]) -> list[bpy.types.Object]:
    meshes = [
        obj
        for obj in bpy.data.objects
        if obj.name not in before_names
        and obj.type == "MESH"
        and not obj.name.startswith("WGT-")
    ]
    return meshes


def import_source_mesh(visual_id: str, source_id: str) -> tuple[bpy.types.Object, Path]:
    source_glb = SOURCE_ROOT / f"{source_id}.glb"
    if not source_glb.exists():
        raise FileNotFoundError(source_glb)

    before = {obj.name for obj in bpy.data.objects}
    bpy.ops.import_scene.gltf(filepath=str(source_glb))
    meshes = imported_meshes(before)
    if not meshes:
        raise RuntimeError(f"{visual_id}: GLB import produced no mesh objects")

    if len(meshes) > 1:
        select_only(meshes)
        bpy.context.view_layer.objects.active = meshes[0]
        bpy.ops.object.join()
        mesh = bpy.context.view_layer.objects.active
    else:
        mesh = meshes[0]

    mesh.name = f"{visual_id}_AnimatedToonStyle_Mesh"
    mesh.data.name = f"{visual_id}_AnimatedToonStyle_Geometry"
    return mesh, source_glb


def first_image_texture_for_mesh(mesh: bpy.types.Object) -> bpy.types.Image | None:
    seen: set[str] = set()
    fallback: bpy.types.Image | None = None
    for slot in mesh.material_slots:
        material = slot.material
        if not material or not material.use_nodes or not material.node_tree:
            continue
        for link in material.node_tree.links:
            if getattr(link.to_node, "type", "") == "BSDF_PRINCIPLED" and link.to_socket.name == "Base Color":
                from_node = link.from_node
                if getattr(from_node, "type", "") == "TEX_IMAGE":
                    image = getattr(from_node, "image", None)
                    if image and (image.has_data or image.packed_file):
                        return image
        for node in material.node_tree.nodes:
            if getattr(node, "type", "") != "TEX_IMAGE":
                continue
            image = getattr(node, "image", None)
            if image and image.name not in seen:
                seen.add(image.name)
                if image.has_data or image.packed_file:
                    fallback = fallback or image
    return fallback


def copy_or_save_image(image: bpy.types.Image, destination: Path) -> bool:
    destination.parent.mkdir(parents=True, exist_ok=True)
    source_path = Path(bpy.path.abspath(image.filepath)) if image.filepath else None
    if source_path and source_path.is_file():
        shutil.copy2(source_path, destination)
        return True

    previous_filepath = image.filepath_raw
    previous_format = image.file_format
    try:
        image.filepath_raw = str(destination)
        image.file_format = "PNG"
        image.save()
        return destination.exists()
    except Exception:
        try:
            image.save_render(filepath=str(destination))
            return destination.exists()
        except Exception:
            return False
    finally:
        image.filepath_raw = previous_filepath
        image.file_format = previous_format


def extracted_texture_sources_for_mesh(visual_id: str, mesh: bpy.types.Object, output_dir: Path) -> dict[str, str]:
    image = first_image_texture_for_mesh(mesh)
    if not image:
        return {}

    output_path = output_dir / "Textures" / f"{visual_id}_BaseColor.png"
    if copy_or_save_image(image, output_path):
        return {"BaseColorTexture": str(output_path)}
    return {}


def record_source_yaw_degrees(source_record: dict[str, str]) -> float:
    return float(source_record.get("source_yaw_degrees", DEFAULT_SOURCE_YAW_DEGREES))


def align_to_template(
    mesh: bpy.types.Object,
    template_mesh: bpy.types.Object,
    source_yaw_degrees: float,
) -> dict[str, float]:
    # Raw Pixal3D humanoid GLBs render back-facing at yaw 0 and front-facing at yaw 180.
    # Normalize GLTF quaternion imports before applying the reviewed source-yaw correction.
    mesh.rotation_mode = "XYZ"
    mesh.rotation_euler[2] += math.radians(source_yaw_degrees)
    apply_object_transform(mesh, location=False, rotation=True, scale=False)

    target_min, target_max = world_bbox(template_mesh)
    source_min, source_max = world_bbox(mesh)
    source_height = max(source_max.z - source_min.z, 0.0001)
    target_height = max(target_max.z - target_min.z, 0.0001)
    scale = target_height / source_height
    mesh.scale = (mesh.scale.x * scale, mesh.scale.y * scale, mesh.scale.z * scale)
    apply_object_transform(mesh, location=False, rotation=False, scale=True)

    source_min, source_max = world_bbox(mesh)
    delta = center_xy(target_min, target_max) - center_xy(source_min, source_max)
    mesh.location += delta
    apply_object_transform(mesh, location=True, rotation=False, scale=False)
    return {
        "template_height": float(target_height),
        "source_height": float(source_height),
        "scale_factor": float(scale),
        "source_yaw_degrees": float(source_yaw_degrees),
    }


def transfer_template_weights(mesh: bpy.types.Object, template_mesh: bpy.types.Object) -> None:
    for group in template_mesh.vertex_groups:
        if mesh.vertex_groups.find(group.name) < 0:
            mesh.vertex_groups.new(name=group.name)

    modifier = mesh.modifiers.new("T66_TemplateWeightTransfer", "DATA_TRANSFER")
    modifier.object = template_mesh
    modifier.use_vert_data = True
    modifier.data_types_verts = {"VGROUP_WEIGHTS"}
    modifier.vert_mapping = "POLYINTERP_NEAREST"
    modifier.layers_vgroup_select_src = "ALL"
    modifier.layers_vgroup_select_dst = "NAME"
    modifier.mix_mode = "REPLACE"
    if hasattr(modifier, "use_create"):
        modifier.use_create = True

    bpy.context.view_layer.objects.active = mesh
    mesh.select_set(True)
    bpy.ops.object.modifier_apply(modifier=modifier.name)


def bind_to_rig(mesh: bpy.types.Object, rig: bpy.types.Object) -> None:
    mesh.parent = rig
    mesh.matrix_parent_inverse = rig.matrix_world.inverted()
    for mod in [m for m in mesh.modifiers if m.type == "ARMATURE"]:
        mesh.modifiers.remove(mod)
    arm_mod = mesh.modifiers.new("Armature", "ARMATURE")
    arm_mod.object = rig
    arm_mod.use_vertex_groups = True


def set_pose_neutral(rig: bpy.types.Object) -> None:
    for bone_name in FK_POSE_BONES:
        pb = rig.pose.bones.get(bone_name)
        if not pb:
            continue
        pb.location = (0.0, 0.0, 0.0)
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.scale = (1.0, 1.0, 1.0)


def set_bone_rotation(rig: bpy.types.Object, bone_name: str, xyz: tuple[float, float, float]) -> None:
    pb = rig.pose.bones.get(bone_name)
    if not pb:
        return
    pb.rotation_mode = "XYZ"
    pb.rotation_euler = xyz


def set_bone_location(rig: bpy.types.Object, bone_name: str, xyz: tuple[float, float, float]) -> None:
    pb = rig.pose.bones.get(bone_name)
    if pb:
        pb.location = xyz


def insert_action_key(
    action: bpy.types.Action,
    rig: bpy.types.Object,
    bone_name: str,
    prop_name: str,
    frame: int,
    values: tuple[float, float, float],
) -> None:
    data_path = f'pose.bones["{bone_name}"].{prop_name}'
    for index, value in enumerate(values):
        fcurve = action.fcurve_ensure_for_datablock(rig, data_path, index=index, group_name=bone_name)
        fcurve.keyframe_points.insert(frame, value, options={"FAST"})


def key_gameplay_pose(
    action: bpy.types.Action,
    rig: bpy.types.Object,
    frame: int,
    transforms: dict[str, dict[str, tuple[float, float, float]]],
) -> None:
    bpy.context.scene.frame_set(frame)
    set_pose_neutral(rig)
    for bone_name, data in transforms.items():
        if "rot" in data:
            set_bone_rotation(rig, bone_name, data["rot"])
        if "loc" in data:
            set_bone_location(rig, bone_name, data["loc"])
    bpy.context.view_layer.update()

    for bone_name in FK_POSE_BONES:
        if bone_name not in rig.pose.bones:
            continue
        data = transforms.get(bone_name, {})
        insert_action_key(action, rig, bone_name, "location", frame, data.get("loc", (0.0, 0.0, 0.0)))
        insert_action_key(action, rig, bone_name, "rotation_euler", frame, data.get("rot", (0.0, 0.0, 0.0)))
        insert_action_key(action, rig, bone_name, "scale", frame, data.get("scale", (1.0, 1.0, 1.0)))


def rebuild_action(rig: bpy.types.Object, action_name: str, frames: list[tuple[int, dict[str, dict[str, tuple[float, float, float]]]]]) -> None:
    existing = bpy.data.actions.get(action_name)
    if existing:
        bpy.data.actions.remove(existing)

    action = bpy.data.actions.new(action_name)
    action.use_fake_user = True
    if rig.animation_data is None:
        rig.animation_data_create()
    rig.animation_data.action = action
    rig.animation_data.use_nla = False

    for frame, transforms in frames:
        key_gameplay_pose(action, rig, frame, transforms)

    for layer in action.layers:
        for strip in layer.strips:
            for channel_bag in strip.channelbags:
                for fcurve in channel_bag.fcurves:
                    fcurve.update()

    action.use_frame_range = True
    action.frame_start = float(frames[0][0])
    action.frame_end = float(frames[-1][0])


def rebuild_conservative_gameplay_actions(rig: bpy.types.Object) -> None:
    """Replace bad named source clips with conservative, non-flipping gameplay clips.

    The earlier template action names are still used by the pipeline, but some
    source clips contained dramatic flip/crouch poses under idle/walk/jump.
    These replacement clips keep the first implementation predictable in-game:
    idle stands, walk steps, jump crouches/lifts/lands. Roll keeps the accepted
    existing somersault source because that clip was reviewed separately.
    """

    rebuild_action(
        rig,
        "Idle_No_Loop",
        [
            (0, {}),
            (40, {}),
            (80, {}),
        ],
    )

    walk_a = {
        "thigh_fk.L": {"rot": (0.34, 0.0, 0.0)},
        "shin_fk.L": {"rot": (-0.20, 0.0, 0.0)},
        "foot_fk.L": {"rot": (0.08, 0.0, 0.0)},
        "thigh_fk.R": {"rot": (-0.34, 0.0, 0.0)},
        "shin_fk.R": {"rot": (0.18, 0.0, 0.0)},
    }
    walk_b = {
        "thigh_fk.L": {"rot": (-0.34, 0.0, 0.0)},
        "shin_fk.L": {"rot": (0.18, 0.0, 0.0)},
        "thigh_fk.R": {"rot": (0.34, 0.0, 0.0)},
        "shin_fk.R": {"rot": (-0.20, 0.0, 0.0)},
        "foot_fk.R": {"rot": (0.08, 0.0, 0.0)},
    }
    rebuild_action(
        rig,
        "Walk_Fwd_Loop_LegsTorsoOnly",
        [
            (0, walk_a),
            (10, {}),
            (20, walk_b),
            (30, {}),
            (40, walk_a),
        ],
    )

    crouch = {
        "root": {"loc": (0.0, 0.0, -0.08)},
        "thigh_fk.L": {"rot": (0.38, 0.0, 0.0)},
        "shin_fk.L": {"rot": (-0.42, 0.0, 0.0)},
        "thigh_fk.R": {"rot": (0.38, 0.0, 0.0)},
        "shin_fk.R": {"rot": (-0.42, 0.0, 0.0)},
    }
    airborne = {
        "root": {"loc": (0.0, 0.0, 0.22)},
        "thigh_fk.L": {"rot": (0.16, 0.0, 0.0)},
        "shin_fk.L": {"rot": (-0.22, 0.0, 0.0)},
        "thigh_fk.R": {"rot": (0.10, 0.0, 0.0)},
        "shin_fk.R": {"rot": (-0.18, 0.0, 0.0)},
    }
    rebuild_action(
        rig,
        "DoubleJump_LegsTorsoOnly",
        [
            (0, {}),
            (6, crouch),
            (14, airborne),
            (22, crouch),
            (28, {}),
        ],
    )

    set_pose_neutral(rig)
    bpy.context.view_layer.update()


def rebuild_static_idle_action(rig: bpy.types.Object) -> None:
    """Use a neutral standing idle; the template's reviewed clips start at Walk."""
    rebuild_action(
        rig,
        "Idle_No_Loop",
        [
            (0, {}),
            (40, {}),
            (80, {}),
        ],
    )


def validate_template_actions(rig: bpy.types.Object) -> dict[str, dict[str, float]]:
    """Confirm the reviewed template actions are present before export."""
    action_report: dict[str, dict[str, float]] = {}
    for label, action_name in ACTION_SOURCES.items():
        action = bpy.data.actions.get(action_name)
        if not action:
            raise RuntimeError(f"Missing reviewed template action {action_name} for {label}")
        start, end = action.frame_range
        action_report[label] = {
            "frame_start": float(start),
            "frame_end": float(end),
            "fcurves": float(sum(
                len(channel_bag.fcurves)
                for layer in action.layers
                for strip in layer.strips
                for channel_bag in strip.channelbags
            )),
        }

    if rig.animation_data:
        rig.animation_data.use_nla = False
    return action_report


def export_mesh(hero_id: str, mesh: bpy.types.Object, rig: bpy.types.Object, hero_dir: Path) -> Path:
    mesh_path = hero_dir / f"{hero_id}_Skeletal.fbx"
    select_only([mesh, rig])
    bpy.ops.export_scene.fbx(
        filepath=str(mesh_path),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=False,
        use_armature_deform_only=True,
        mesh_smooth_type="FACE",
        path_mode="COPY",
        embed_textures=True,
    )
    return mesh_path


def export_animation(hero_id: str, label: str, action_name: str, rig: bpy.types.Object, hero_dir: Path) -> Path:
    action = bpy.data.actions.get(action_name)
    if not action:
        raise RuntimeError(f"Missing template action {action_name}")

    if rig.animation_data is None:
        rig.animation_data_create()
    rig.animation_data.action = action
    if getattr(action, "slots", None) and len(action.slots) > 0:
        rig.animation_data.action_slot = action.slots[0]
    rig.animation_data.use_nla = False

    start, end = action.frame_range
    bpy.context.scene.frame_start = int(math.floor(start))
    bpy.context.scene.frame_end = int(math.ceil(end))
    bpy.context.scene.frame_set(bpy.context.scene.frame_start)

    anim_path = hero_dir / f"{hero_id}_{label}.fbx"
    select_only([rig])
    bpy.ops.export_scene.fbx(
        filepath=str(anim_path),
        use_selection=True,
        object_types={"ARMATURE"},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False,
        bake_anim_force_startend_keying=True,
        use_armature_deform_only=True,
    )
    return anim_path


def texture_sources_for_hero(hero_id: str) -> dict[str, str]:
    run_root = TEMPLATE_BLEND.parents[1]
    working_root = run_root / "Processed" / hero_id / "Working"
    candidates = {
        "BaseColorTexture": working_root / f"{hero_id}_0.png",
        "TintTexture": working_root / f"{hero_id}_Tint.png",
        "InnerLineTexture": working_root / f"{hero_id}_InnerLines.png",
    }

    sources = {
        parameter: str(path)
        for parameter, path in candidates.items()
        if path.exists()
    }
    if hero_id == "Hero_1_Chad" and "BaseColorTexture" not in sources:
        accepted_texture = (
            run_root
            / "AccuRig_Textured"
            / hero_id
            / f"{hero_id}_Textured.fbm"
            / f"{hero_id}_00_Image_0.png"
        )
        if accepted_texture.exists():
            sources["BaseColorTexture"] = str(accepted_texture)
    return sources


def hero_record(hero_id: str) -> dict[str, str]:
    _, index, body = hero_id.split("_", 2)
    return {
        "visual_id": hero_id,
        "source_id": hero_id,
        "target_dir": f"/Game/Characters/Heroes/Hero_{index}/{body}/AnimatedToonStyle",
        "mesh_relative_scale": "(X=1,Y=1,Z=1)",
        "asset_kind": "hero",
    }


def requested_records() -> list[dict[str, str]]:
    if REQUESTED_RECORDS_JSON:
        records = json.loads(REQUESTED_RECORDS_JSON)
        if not isinstance(records, list):
            raise RuntimeError("T66_ANIMATED_TOONSTYLE_RECORDS_JSON must be a JSON list")
        return records

    if REQUESTED_HEROES:
        unknown_heroes = sorted(set(REQUESTED_HEROES) - set(DEFAULT_HEROES))
        if unknown_heroes:
            raise RuntimeError(f"Unknown hero ids requested: {unknown_heroes}")
        return [hero_record(hero_id) for hero_id in REQUESTED_HEROES]

    if SOURCE_PRESET == "humanoid_guideline_20260522":
        return list(HUMANOID_GUIDELINE_RECORDS)

    if SOURCE_PRESET not in {"", "default_heroes"}:
        raise RuntimeError(f"Unknown T66_ANIMATED_TOONSTYLE_SOURCE_PRESET: {SOURCE_PRESET}")
    return [hero_record(hero_id) for hero_id in DEFAULT_HEROES]


def visible_for_export(obj: bpy.types.Object) -> tuple[bool, bool]:
    hidden = obj.hide_get()
    hidden_render = obj.hide_render
    obj.hide_set(False)
    obj.hide_render = False
    return hidden, hidden_render


def restore_visibility(obj: bpy.types.Object, state: tuple[bool, bool]) -> None:
    hidden, hidden_render = state
    obj.hide_set(hidden)
    obj.hide_render = hidden_render


def main() -> None:
    if not TEMPLATE_BLEND.exists():
        raise FileNotFoundError(TEMPLATE_BLEND)

    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=str(TEMPLATE_BLEND))

    rig = bpy.data.objects.get(TEMPLATE_RIG_NAME)
    template_mesh = bpy.data.objects.get(TEMPLATE_MESH_NAME)
    if not rig or rig.type != "ARMATURE":
        raise RuntimeError(f"Missing template rig {TEMPLATE_RIG_NAME}")
    if not template_mesh or template_mesh.type != "MESH":
        raise RuntimeError(f"Missing template mesh {TEMPLATE_MESH_NAME}")

    rebuild_static_idle_action(rig)
    action_report = validate_template_actions(rig)

    records = []
    source_records = requested_records()

    template_visibility = visible_for_export(template_mesh)

    for source_record in source_records:
        visual_id = str(source_record["visual_id"])
        source_id = str(source_record.get("source_id") or visual_id)
        character_dir = OUTPUT_ROOT / visual_id
        character_dir.mkdir(parents=True, exist_ok=True)

        extracted_texture_sources = {}
        if visual_id in EXACT_TEMPLATE_HERO_IDS and source_id == visual_id:
            mesh = template_mesh
            source_path = TEMPLATE_BLEND
            alignment = {"mode": "accepted_template_mesh", "source_yaw_degrees": 0.0}
            mesh_path = export_mesh(visual_id, mesh, rig, character_dir)
        else:
            mesh, source_path = import_source_mesh(visual_id, source_id)
            source_yaw_degrees = record_source_yaw_degrees(source_record)
            alignment = align_to_template(mesh, template_mesh, source_yaw_degrees)
            transfer_template_weights(mesh, template_mesh)
            bind_to_rig(mesh, rig)
            extracted_texture_sources = extracted_texture_sources_for_mesh(visual_id, mesh, character_dir)
            mesh_path = export_mesh(visual_id, mesh, rig, character_dir)

        animations = {}
        for label, action_name in ACTION_SOURCES.items():
            animations[label] = str(export_animation(visual_id, label, action_name, rig, character_dir))

        texture_sources = texture_sources_for_hero(visual_id)
        for parameter_name, texture_source in extracted_texture_sources.items():
            texture_sources.setdefault(parameter_name, texture_source)

        records.append(
            {
                "hero_id": visual_id,
                "visual_id": visual_id,
                "source_id": source_id,
                "source_glb": str(source_path),
                "mesh_fbx": str(mesh_path),
                "animations": animations,
                "alignment": alignment,
                "preserve_source_materials": False,
                "texture_sources": texture_sources,
                "template_actions": action_report,
                "target_dir": source_record.get("target_dir"),
                "asset_kind": source_record.get("asset_kind", "hero"),
                "mesh_relative_location": source_record.get("mesh_relative_location", "(X=0,Y=0,Z=0)"),
                "mesh_relative_rotation": source_record.get("mesh_relative_rotation", "(Pitch=0,Yaw=-90.000000,Roll=0)"),
                "mesh_relative_scale": source_record.get("mesh_relative_scale", "(X=1,Y=1,Z=1)"),
                "b_auto_ground_to_actor_origin": source_record.get("b_auto_ground_to_actor_origin", "true"),
            }
        )

        if not (visual_id in EXACT_TEMPLATE_HERO_IDS and source_id == visual_id):
            bpy.data.objects.remove(mesh, do_unlink=True)

    restore_visibility(template_mesh, template_visibility)

    manifest = {
        "pipeline": "AnimatedToonStyleHeroRigifyTemplate",
        "template_blend": str(TEMPLATE_BLEND),
        "source_root": str(SOURCE_ROOT),
        "output_root": str(OUTPUT_ROOT),
        "source_preset": SOURCE_PRESET,
        "heroes": records,
        "characters": records,
    }
    manifest_path = OUTPUT_ROOT / "animated_toonstyle_hero_sources_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"[AnimatedToonStyleHeroes] wrote {manifest_path}")


if __name__ == "__main__":
    main()
