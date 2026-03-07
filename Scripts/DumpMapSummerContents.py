import unreal
import os

MAP_PATH = "/Game/LowPolyNature/Maps/Map_Summer"
OUTPUT_FILE = os.path.join(unreal.Paths.project_saved_dir(), "Logs", "Map_Summer_Dump.txt")

editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = editor_subsystem.get_editor_world()

current_map = world.get_path_name() if world else ""
need_load = MAP_PATH not in current_map

if need_load:
    unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

actors = unreal.EditorLevelLibrary.get_all_level_actors()

lines = []
lines.append("=" * 80)
lines.append(f"Map_Summer Actor Dump  ({len(actors)} actors)")
lines.append("=" * 80)

static_meshes = {}
lights = []
other_actors = []

for actor in actors:
    class_name = actor.get_class().get_name()
    actor_label = actor.get_actor_label()
    loc = actor.get_actor_location()
    rot = actor.get_actor_rotation()
    scale = actor.get_actor_scale3d()

    if class_name == "StaticMeshActor":
        comp = actor.static_mesh_component
        if comp and comp.static_mesh:
            mesh_path = comp.static_mesh.get_path_name()
            mesh_name = comp.static_mesh.get_name()
            mats = []
            for i in range(comp.get_num_materials()):
                mat = comp.get_material(i)
                mats.append(mat.get_name() if mat else "None")

            entry = {
                "label": actor_label,
                "mesh": mesh_name,
                "mesh_path": mesh_path,
                "materials": mats,
                "location": f"({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})",
                "rotation": f"({rot.pitch:.1f}, {rot.yaw:.1f}, {rot.roll:.1f})",
                "scale": f"({scale.x:.2f}, {scale.y:.2f}, {scale.z:.2f})",
            }
            static_meshes.setdefault(mesh_name, []).append(entry)
        else:
            other_actors.append((class_name, actor_label, loc))
    elif "Light" in class_name:
        lights.append((class_name, actor_label, loc))
    else:
        other_actors.append((class_name, actor_label, loc))

lines.append("")
lines.append("-" * 80)
lines.append("STATIC MESHES (grouped by mesh)")
lines.append("-" * 80)
for mesh_name in sorted(static_meshes.keys()):
    instances = static_meshes[mesh_name]
    lines.append(f"\n  {mesh_name}  x{len(instances)}")
    lines.append(f"    Path: {instances[0]['mesh_path']}")
    lines.append(f"    Materials: {', '.join(instances[0]['materials'])}")
    for inst in instances:
        lines.append(f"      Loc={inst['location']}  Rot={inst['rotation']}  Scale={inst['scale']}")

lines.append("")
lines.append("-" * 80)
lines.append(f"LIGHTS ({len(lights)})")
lines.append("-" * 80)
for cls, label, loc in lights:
    lines.append(f"  {cls}: {label}  Loc=({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})")

lines.append("")
lines.append("-" * 80)
lines.append(f"OTHER ACTORS ({len(other_actors)})")
lines.append("-" * 80)
for cls, label, loc in other_actors:
    lines.append(f"  {cls}: {label}  Loc=({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})")

# Summary
lines.append("")
lines.append("=" * 80)
lines.append("SUMMARY")
lines.append("=" * 80)
lines.append(f"Total actors: {len(actors)}")
lines.append(f"Unique meshes: {len(static_meshes)}")
lines.append(f"Total static mesh instances: {sum(len(v) for v in static_meshes.values())}")
lines.append(f"Lights: {len(lights)}")
lines.append(f"Other actors: {len(other_actors)}")
lines.append("")
lines.append("Mesh instance counts:")
for mesh_name in sorted(static_meshes.keys(), key=lambda m: -len(static_meshes[m])):
    lines.append(f"  {len(static_meshes[mesh_name]):4d}x  {mesh_name}")

output = "\n".join(lines)

os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)
with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
    f.write(output)

unreal.log(f"Map_Summer dump written to: {OUTPUT_FILE}")
unreal.log(f"Total actors: {len(actors)}, Unique meshes: {len(static_meshes)}")
