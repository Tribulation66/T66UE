import json
from pathlib import Path

import unreal


ASSET_PATH = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/PA_Hero_1_Chad_PhysicsFirst_Stage2Seed"
OUTPUT_PATH = Path(r"C:\UE\T66\Reports\AgentReviews\HeroRagdollFlatteningFix\physicsfirst_asset_before.json")


def prop(obj, name):
    try:
        value = obj.get_editor_property(name)
    except Exception as exc:
        return f"<unavailable: {exc}>"
    return serialize(value)


def serialize(value):
    if isinstance(value, unreal.Name):
        return str(value)
    if isinstance(value, (str, int, float, bool)) or value is None:
        return value
    if hasattr(value, "x") and hasattr(value, "y") and hasattr(value, "z"):
        return [float(value.x), float(value.y), float(value.z)]
    if hasattr(value, "roll") and hasattr(value, "pitch") and hasattr(value, "yaw"):
        return [float(value.roll), float(value.pitch), float(value.yaw)]
    if hasattr(value, "name"):
        return str(value.name)
    return str(value)


def array_prop(obj, *names):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            pass
        try:
            return getattr(obj, name)
        except Exception:
            pass
    return None


def primitive_rows(agg, prop_name, fields):
    rows = []
    elems = array_prop(agg, prop_name)
    if elems is None:
        return rows
    for elem in elems:
        row = {}
        for field in fields:
            row[field] = prop(elem, field)
        rows.append(row)
    return rows


asset = unreal.load_asset(ASSET_PATH)
if not asset:
    raise RuntimeError(f"Could not load {ASSET_PATH}")

body_setups = array_prop(asset, "skeletal_body_setups", "SkeletalBodySetups")
constraint_setups = array_prop(asset, "constraint_setup", "ConstraintSetup")
if body_setups is None or constraint_setups is None:
    payload = {
        "ok": False,
        "asset": ASSET_PATH,
        "error": "PhysicsAsset arrays are not exposed through UE Python",
        "available_members": [
            name for name in dir(asset)
            if "body" in name.lower() or "constraint" in name.lower() or "collision" in name.lower()
        ],
    }
    OUTPUT_PATH.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"Wrote limited dump {OUTPUT_PATH}")
    unreal.SystemLibrary.quit_editor()
    raise SystemExit(0)

bodies = []
for body in body_setups:
    instance = body.get_editor_property("default_instance")
    agg = body.get_editor_property("agg_geom")
    row = {
        "bone_name": str(body.get_editor_property("bone_name")),
        "physics_type": prop(instance, "physics_type"),
        "linear_damping": prop(instance, "linear_damping"),
        "angular_damping": prop(instance, "angular_damping"),
        "mass_scale": prop(instance, "mass_scale"),
        "mass_in_kg": prop(instance, "mass_in_kg"),
        "override_mass": prop(instance, "override_mass"),
        "start_awake": prop(instance, "start_awake"),
        "enable_gravity": prop(instance, "enable_gravity"),
        "use_ccd": prop(instance, "use_ccd"),
        "sleep_family": prop(instance, "sleep_family"),
        "stabilization_threshold_multiplier": prop(instance, "stabilization_threshold_multiplier"),
        "inertia_tensor_scale": prop(instance, "inertia_tensor_scale"),
        "position_solver_iteration_count": prop(instance, "position_solver_iteration_count"),
        "velocity_solver_iteration_count": prop(instance, "velocity_solver_iteration_count"),
        "projection_solver_iteration_count": prop(instance, "projection_solver_iteration_count"),
        "sphyls": primitive_rows(agg, "sphyl_elems", ["radius", "length", "center", "rotation"]),
        "spheres": primitive_rows(agg, "sphere_elems", ["radius", "center"]),
        "boxes": primitive_rows(agg, "box_elems", ["x", "y", "z", "center", "rotation"]),
        "convex_count": len(array_prop(agg, "convex_elems") or []),
    }
    bodies.append(row)

constraints = []
for template in constraint_setups:
    ci = template.get_editor_property("default_instance")
    constraints.append({
        "constraint_bone1": prop(ci, "constraint_bone1"),
        "constraint_bone2": prop(ci, "constraint_bone2"),
        "joint_name": prop(ci, "joint_name"),
        "linear_x_motion": prop(ci, "linear_x_motion"),
        "linear_y_motion": prop(ci, "linear_y_motion"),
        "linear_z_motion": prop(ci, "linear_z_motion"),
        "linear_limit_size": prop(ci, "linear_limit_size"),
        "angular_swing1_motion": prop(ci, "angular_swing1_motion"),
        "angular_swing2_motion": prop(ci, "angular_swing2_motion"),
        "angular_twist_motion": prop(ci, "angular_twist_motion"),
        "angular_swing1_limit": prop(ci, "angular_swing1_limit"),
        "angular_swing2_limit": prop(ci, "angular_swing2_limit"),
        "angular_twist_limit": prop(ci, "angular_twist_limit"),
        "disable_collision": prop(ci, "disable_collision"),
        "parent_dominates": prop(ci, "parent_dominates"),
        "enable_projection": prop(ci, "enable_projection"),
        "projection_linear_alpha": prop(ci, "projection_linear_alpha"),
        "projection_angular_alpha": prop(ci, "projection_angular_alpha"),
        "shock_propagation_alpha": prop(ci, "shock_propagation_alpha"),
        "profile_instance": prop(ci, "profile_instance"),
    })

payload = {
    "ok": True,
    "asset": ASSET_PATH,
    "body_count": len(bodies),
    "constraint_count": len(constraints),
    "bodies": bodies,
    "constraints": constraints,
}

OUTPUT_PATH.write_text(json.dumps(payload, indent=2), encoding="utf-8")
print(f"Wrote {OUTPUT_PATH}")
unreal.SystemLibrary.quit_editor()
