import importlib.util
import json
import math
import re
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


MODEL_GEN_ROOT = Path(__file__).resolve().parents[1]
BATCH_ROOT = MODEL_GEN_ROOT / "Runs" / "Heroes" / "TypeA_Masculine_Batch01"
SOURCE_DIR = BATCH_ROOT / "Assembly" / "HeadBody"
RIG_ROOT = BATCH_ROOT / "Rigging"
SCENE_ROOT = MODEL_GEN_ROOT / "Scenes" / "TypeA_Batch01_Rigging"
IMPORT_SCRIPT = Path(__file__).with_name("rig_typea_mike_prototype.py")

SOURCE_RE = re.compile(r"^Hero_(?P<id>\d+)_(?P<name>.+)_TypeA_(?P<skin>Standard|BeachGoer)_HeadBody_Assembled\.glb$")
ACTIVE_HERO_IDS = {1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 15}
TARGET_UNREAL_HEIGHT_M = 2.0

VIEW_YAWS = {
    "front": 0.0,
    "right": 90.0,
    "left_oblique": -35.0,
}


def load_prototype_module():
    spec = importlib.util.spec_from_file_location("rig_typea_mike_prototype", str(IMPORT_SCRIPT))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


rig = load_prototype_module()


def discover_sources():
    sources = []
    for path in sorted(SOURCE_DIR.glob("*_HeadBody_Assembled.glb")):
        match = SOURCE_RE.match(path.name)
        if not match:
            continue
        hero_id = int(match.group("id"))
        if hero_id not in ACTIVE_HERO_IDS:
            continue
        skin_key = match.group("skin")
        sources.append(
            {
                "hero_id": hero_id,
                "hero_name": match.group("name"),
                "skin": "Beachgoer" if skin_key == "BeachGoer" else "Standard",
                "source": path,
            }
        )
    return sources


def safe_name(value):
    return re.sub(r"[^A-Za-z0-9_]+", "_", value).strip("_")


def output_meta(item):
    hero_slug = f"Hero_{item['hero_id']}_{safe_name(item['hero_name'])}"
    skin = item["skin"]
    asset_suffix = "" if skin == "Standard" else "_Beachgoer"
    asset_base = f"Hero_{item['hero_id']}_TypeA{asset_suffix}"
    short_skin = "Beach" if skin != "Standard" else "Std"
    return {
        "hero_slug": hero_slug,
        "skin": skin,
        "asset_base": asset_base,
        "mesh_asset": f"SK_{asset_base}",
        "idle_asset": f"AM_{asset_base}_Idle",
        "short_rig": f"Rig_H{item['hero_id']}_TA_{short_skin}",
        "out_root": RIG_ROOT / hero_slug / skin,
        "scene": SCENE_ROOT / f"{hero_slug}_TypeA_{skin}_Rigged.blend",
        "unreal_dest": f"/Game/Characters/Heroes/Hero_{item['hero_id']}/TypeA" + ("/Beachgoer" if skin != "Standard" else ""),
    }


def import_source(item, meta):
    bpy.ops.import_scene.gltf(filepath=str(item["source"]))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(meshes) != 2:
        raise RuntimeError(f"Expected 2 meshes from {item['source'].name}, got {len(meshes)}")

    meshes_by_world_min_z = sorted(meshes, key=lambda obj: rig.object_bounds([obj])[0].z)
    body = meshes_by_world_min_z[0]
    head = meshes_by_world_min_z[-1]
    body.name = f"{meta['hero_slug']}_{meta['skin']}_Body"
    body.data.name = f"{body.name}_Mesh"
    head.name = f"{meta['hero_slug']}_{meta['skin']}_Head"
    head.data.name = f"{head.name}_Mesh"

    for obj in (body, head):
        world = obj.matrix_world.copy()
        obj.parent = None
        obj.matrix_world = Matrix.Identity(4)
        obj.data.transform(world)
        obj.matrix_world = Matrix.Identity(4)
        obj.data.update()

    for obj in list(bpy.context.scene.objects):
        if obj.type == "EMPTY" and obj.name.endswith("_world"):
            bpy.data.objects.remove(obj, do_unlink=True)
    return body, head


def vector_to_list(value):
    return [float(value.x), float(value.y), float(value.z)]


def mesh_vertex_bounds(objects):
    min_v = Vector((math.inf, math.inf, math.inf))
    max_v = Vector((-math.inf, -math.inf, -math.inf))
    for obj in objects:
        if obj.type != "MESH":
            continue
        world = obj.matrix_world
        for vertex in obj.data.vertices:
            point = world @ vertex.co
            min_v.x = min(min_v.x, point.x)
            min_v.y = min(min_v.y, point.y)
            min_v.z = min(min_v.z, point.z)
            max_v.x = max(max_v.x, point.x)
            max_v.y = max(max_v.y, point.y)
            max_v.z = max(max_v.z, point.z)
    return min_v, max_v, max_v - min_v


def normalize_to_feet_origin(body, head):
    original_min, original_max, original_size = mesh_vertex_bounds([body, head])
    if original_size.z <= 0.001:
        raise RuntimeError("Cannot normalize Type A rig source with zero height")

    foot_center = Vector((
        (original_min.x + original_max.x) * 0.5,
        (original_min.y + original_max.y) * 0.5,
        original_min.z,
    ))
    scale = TARGET_UNREAL_HEIGHT_M / original_size.z
    transform = Matrix.Scale(scale, 4) @ Matrix.Translation(-foot_center)

    for obj in (body, head):
        obj.data.transform(transform)
        obj.data.update()
    bpy.context.view_layer.update()

    normalized_min, normalized_max, normalized_size = mesh_vertex_bounds([body, head])
    return {
        "target_height_m": TARGET_UNREAL_HEIGHT_M,
        "source_min_m": vector_to_list(original_min),
        "source_max_m": vector_to_list(original_max),
        "source_size_m": vector_to_list(original_size),
        "source_foot_center_m": vector_to_list(foot_center),
        "baked_scale": float(scale),
        "normalized_min_m": vector_to_list(normalized_min),
        "normalized_max_m": vector_to_list(normalized_max),
        "normalized_size_m": vector_to_list(normalized_size),
    }


def rename_armature(armature, meta):
    armature.name = meta["short_rig"]
    armature.data.name = f"{armature.name}_Data"


def reset_pose(armature):
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.location = (0.0, 0.0, 0.0)
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def set_idle_pose(armature, phase):
    reset_pose(armature)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="POSE")

    p = float(phase)
    bones = armature.pose.bones
    bones["pelvis"].rotation_euler = (0.0, math.radians(0.6 * p), math.radians(0.4 * p))
    bones["spine_01"].rotation_euler = (0.0, math.radians(-0.8 * p), math.radians(-0.5 * p))
    bones["spine_02"].rotation_euler = (0.0, math.radians(-0.6 * p), math.radians(-0.4 * p))
    bones["upperarm_l"].rotation_euler = (math.radians(3.0 * p), 0.0, math.radians(-7.0 * p))
    bones["lowerarm_l"].rotation_euler = (0.0, 0.0, math.radians(-3.0 * p))
    bones["hand_l"].rotation_euler = (0.0, 0.0, math.radians(-1.5 * p))
    bones["upperarm_r"].rotation_euler = (math.radians(-3.0 * p), 0.0, math.radians(7.0 * p))
    bones["lowerarm_r"].rotation_euler = (0.0, 0.0, math.radians(3.0 * p))
    bones["hand_r"].rotation_euler = (0.0, 0.0, math.radians(1.5 * p))

    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.update()


def insert_pose_keys(armature, frame):
    bpy.ops.object.mode_set(mode="POSE")
    for pb in armature.pose.bones:
        pb.keyframe_insert("rotation_euler", frame=frame)
        pb.keyframe_insert("location", frame=frame)
    bpy.ops.object.mode_set(mode="OBJECT")


def build_idle_action(armature, meta):
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 48
    action = bpy.data.actions.new(meta["idle_asset"])
    action.use_fake_user = True
    armature.animation_data_create()
    armature.animation_data.action = action

    for frame, phase in ((1, -1.0), (12, 0.0), (24, 1.0), (36, 0.0), (48, -1.0)):
        scene.frame_set(frame)
        set_idle_pose(armature, phase)
        insert_pose_keys(armature, frame)

    armature.animation_data.action = action
    scene.frame_set(1)
    set_idle_pose(armature, -1.0)
    return action


def add_camera(center, size, yaw_deg, pitch_deg=4.0, scale=1.22):
    cam_data = bpy.data.cameras.new(name=f"RigQA_Camera_{yaw_deg:g}")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new(cam_data.name, cam_data)
    bpy.context.scene.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    direction = Vector((math.sin(yaw) * math.cos(pitch), -math.cos(yaw) * math.cos(pitch), math.sin(pitch)))
    cam.location = center + direction * 4.0
    target = bpy.data.objects.new(f"RigQA_Target_{yaw_deg:g}", None)
    target.location = center
    bpy.context.scene.collection.objects.link(target)
    track = cam.constraints.new(type="TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    cam_data.ortho_scale = max(size.x, size.z) * scale


def clear_qa_cameras():
    for obj in list(bpy.data.objects):
        if obj.name.startswith("RigQA_Camera_") or obj.name.startswith("RigQA_Target_"):
            bpy.data.objects.remove(obj, do_unlink=True)


def render_view(meta, label, yaw, pose_label, resolution=760, scale=1.22):
    render_dir = meta["out_root"] / "Renders"
    render_dir.mkdir(parents=True, exist_ok=True)
    min_v, max_v, size = rig.all_mesh_bounds()
    center = (min_v + max_v) * 0.5
    clear_qa_cameras()
    add_camera(center, size, yaw, scale=scale)
    scene = bpy.context.scene
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    out = render_dir / f"{meta['asset_base']}_{pose_label}_{label}.png"
    scene.render.filepath = str(out)
    bpy.ops.render.render(write_still=True)
    return out


def sanitize_unreal_asset_name(value):
    return safe_name(value.replace(".", "_"))


def first_material_image(material):
    if not material or not material.node_tree:
        return None

    image_nodes = [
        node
        for node in material.node_tree.nodes
        if getattr(node, "image", None) is not None
    ]
    if not image_nodes:
        return None

    def image_score(node):
        name = f"{node.name} {node.label} {node.image.name}".lower()
        if "normal" in name or "rough" in name or "metal" in name:
            return 10
        if "image_0" in name or "base" in name or "diffuse" in name or "albedo" in name:
            return -10
        return 0

    image_nodes.sort(key=image_score)
    return image_nodes[0].image


def export_base_color_textures(meshes, meta):
    texture_dir = meta["out_root"] / "Textures"
    texture_dir.mkdir(parents=True, exist_ok=True)
    exported = []
    used_texture_names = set()

    for obj in meshes:
        for slot_index, slot in enumerate(obj.material_slots):
            material = slot.material
            image = first_material_image(material)
            if not image:
                continue

            material_name = sanitize_unreal_asset_name(material.name if material else slot.name or f"Material_{slot_index}")
            texture_name = sanitize_unreal_asset_name(image.name or f"Image_{slot_index}")
            if texture_name in used_texture_names:
                texture_name = f"{texture_name}_{slot_index}"
            used_texture_names.add(texture_name)

            out_path = texture_dir / f"{texture_name}.png"
            image.save_render(filepath=str(out_path))
            exported.append(
                {
                    "source_object": obj.name,
                    "slot_index": slot_index,
                    "slot_name": slot.name,
                    "material_name": material_name,
                    "texture_asset_name": texture_name,
                    "base_color_png": str(out_path),
                }
            )

    return exported


def export_outputs(armature, meta):
    export_dir = meta["out_root"] / "Exports"
    export_dir.mkdir(parents=True, exist_ok=True)
    glb_path = export_dir / f"{meta['asset_base']}_Rigged.glb"
    fbx_path = export_dir / f"{meta['asset_base']}_Rigged.fbx"

    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    for obj in bpy.context.scene.objects:
        if obj.type == "MESH":
            obj.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.gltf(
        filepath=str(glb_path),
        export_format="GLB",
        use_selection=True,
        export_animations=True,
        export_materials="EXPORT",
    )
    bpy.ops.export_scene.fbx(
        filepath=str(fbx_path),
        use_selection=True,
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=True,
        object_types={"ARMATURE", "MESH"},
        path_mode="COPY",
        embed_textures=True,
    )
    return glb_path, fbx_path


def run_one(item):
    meta = output_meta(item)
    meta["out_root"].mkdir(parents=True, exist_ok=True)
    meta["scene"].parent.mkdir(parents=True, exist_ok=True)

    rig.OUT_ROOT = meta["out_root"]
    rig.SCENE_OUT = meta["scene"]
    rig.reset_scene()
    body, head = import_source(item, meta)
    normalization = normalize_to_feet_origin(body, head)
    armature, points = rig.create_armature(body, head)
    rename_armature(armature, meta)
    rig.weight_body(body)
    stabilized_components = rig.stabilize_small_components(body, max_vertices=100)
    rig.weight_head(head)
    rig.attach_armature(body, armature)
    rig.attach_armature(head, armature)
    rig.add_lights()
    texture_exports = export_base_color_textures((body, head), meta)

    idle_action = build_idle_action(armature, meta)
    render_outputs = {}
    armature.animation_data.action = None

    bpy.context.scene.frame_set(1)
    reset_pose(armature)
    for view, yaw in VIEW_YAWS.items():
        render_outputs[f"rest_{view}"] = str(render_view(meta, view, yaw, "rest"))

    bpy.context.scene.frame_set(1)
    set_idle_pose(armature, -1.0)
    for view, yaw in VIEW_YAWS.items():
        render_outputs[f"idle_left_{view}"] = str(render_view(meta, view, yaw, "idle_left"))

    bpy.context.scene.frame_set(24)
    set_idle_pose(armature, 1.0)
    for view, yaw in VIEW_YAWS.items():
        render_outputs[f"idle_right_{view}"] = str(render_view(meta, view, yaw, "idle_right"))

    armature.animation_data.action = idle_action
    glb_path, fbx_path = export_outputs(armature, meta)
    bpy.ops.wm.save_as_mainfile(filepath=str(meta["scene"]))

    report = {
        "hero_id": item["hero_id"],
        "hero_name": item["hero_name"],
        "skin": item["skin"],
        "source": str(item["source"]),
        "blend": str(meta["scene"]),
        "exports": {"glb": str(glb_path), "fbx": str(fbx_path)},
        "renders": render_outputs,
        "unreal": {
            "destination": meta["unreal_dest"],
            "skeletal_mesh": f"{meta['unreal_dest']}/{meta['mesh_asset']}.{meta['mesh_asset']}",
            "idle_animation": f"{meta['unreal_dest']}/{meta['idle_asset']}.{meta['idle_asset']}",
            "import_uniform_scale": 1.0,
        },
        "normalization": normalization,
        "textures": texture_exports,
        "armature": armature.name,
        "bones": [bone.name for bone in armature.data.bones],
        "deform_bones": rig.DEFORM_BONES,
        "mesh_vertex_groups": {
            obj.name: [group.name for group in obj.vertex_groups]
            for obj in bpy.context.scene.objects
            if obj.type == "MESH"
        },
        "body_small_components_stabilized": stabilized_components,
        "animation": {
            "idle_action": idle_action.name,
            "frame_start": bpy.context.scene.frame_start,
            "frame_end": bpy.context.scene.frame_end,
            "actions": [action.name for action in bpy.data.actions],
        },
    }
    report_path = meta["out_root"] / "rig_report.json"
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    return report


def verify_fbx(report):
    fbx_path = report["exports"]["fbx"]
    rig.reset_scene()
    bpy.ops.import_scene.fbx(filepath=fbx_path)
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    actions = [action.name for action in bpy.data.actions]
    skinned_meshes = [
        obj.name
        for obj in meshes
        if any(mod.type == "ARMATURE" and mod.object in armatures for mod in obj.modifiers)
    ]
    return {
        "fbx": fbx_path,
        "armature_count": len(armatures),
        "bone_count": len(armatures[0].data.bones) if armatures else 0,
        "mesh_count": len(meshes),
        "skinned_mesh_count": len(skinned_meshes),
        "actions": actions,
        "ok": len(armatures) == 1
        and len(armatures[0].data.bones) == 18
        and len(meshes) == 2
        and len(skinned_meshes) == 2
        and any(report["animation"]["idle_action"] in action for action in actions),
    }


def main():
    sources = discover_sources()
    if len(sources) != 24:
        raise RuntimeError(f"Expected 24 active TypeA assembled sources, found {len(sources)}")

    reports = []
    failures = []
    for index, item in enumerate(sources, start=1):
        label = f"Hero_{item['hero_id']}_{item['hero_name']}_{item['skin']}"
        print(f"[{index:02d}/{len(sources)}] Rigging {label}")
        try:
            reports.append(run_one(item))
        except Exception as exc:
            failures.append({"source": str(item["source"]), "error": repr(exc)})
            print(f"[ERROR] {label}: {exc!r}")

    verifications = []
    if not failures:
        for report in reports:
            check = verify_fbx(report)
            verifications.append(check)
            print(f"[VERIFY] {Path(check['fbx']).name}: ok={check['ok']} bones={check['bone_count']} meshes={check['mesh_count']} actions={check['actions']}")

    batch_report = {
        "source_count": len(sources),
        "rigged_count": len(reports),
        "failure_count": len(failures),
        "failures": failures,
        "reports": reports,
        "verifications": verifications,
        "all_verified": bool(reports) and not failures and all(v["ok"] for v in verifications),
    }
    report_path = RIG_ROOT / "batch_typea_rig_report.json"
    report_path.write_text(json.dumps(batch_report, indent=2), encoding="utf-8")
    print(json.dumps({"batch_report": str(report_path), "all_verified": batch_report["all_verified"]}, indent=2))
    if failures or not batch_report["all_verified"]:
        raise SystemExit(2)


if __name__ == "__main__":
    main()
