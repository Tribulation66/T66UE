import json
from pathlib import Path

import unreal


ASSET_PATH = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/PA_Hero_1_Chad_Male_FriendSlop_TestRoom"
OUTPUT_PATH = Path(r"C:\UE\T66\Reports\AgentReviews\FriendSlopRagdollReassessment\friendslop_current_physics_asset_dump.json")


def get_prop(obj, name):
    try:
        value = obj.get_editor_property(name)
        if isinstance(value, unreal.Name):
            return str(value)
        if hasattr(value, "name"):
            return str(value.name)
        if isinstance(value, (int, float, str, bool)) or value is None:
            return value
        return str(value)
    except Exception as exc:
        return f"<unavailable: {exc}>"


def vec_to_list(value):
    try:
        return [float(value.x), float(value.y), float(value.z)]
    except Exception:
        return str(value)


def rot_to_list(value):
    try:
        return [float(value.roll), float(value.pitch), float(value.yaw)]
    except Exception:
        return str(value)


asset = unreal.load_asset(ASSET_PATH)
if not asset:
    raise RuntimeError(f"Could not load {ASSET_PATH}")

available_members = [name for name in dir(asset) if "body" in name.lower() or "constraint" in name.lower() or "physics" in name.lower()]


def get_array(obj, *names):
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

body_rows = []
body_setups = get_array(asset, "skeletal_body_setups", "SkeletalBodySetups")
constraint_setups = get_array(asset, "constraint_setup", "ConstraintSetup")
if body_setups is None or constraint_setups is None:
    exposed_constraints = []
    try:
        for item in asset.get_constraints(True):
            exposed_constraints.append({
                "type": str(type(item)),
                "repr": str(item),
                "members": [name for name in dir(item) if "bone" in name.lower() or "limit" in name.lower() or "motion" in name.lower() or "constraint" in name.lower()],
            })
    except Exception as exc:
        exposed_constraints.append({"error": str(exc)})
    payload = {
        "asset": ASSET_PATH,
        "error": "UE Python did not expose skeletal body/constraint arrays for PhysicsAsset",
        "available_members": available_members,
        "get_constraints": exposed_constraints,
    }
    OUTPUT_PATH.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"Wrote limited API dump {OUTPUT_PATH}")
    unreal.SystemLibrary.quit_editor()
    raise SystemExit(0)

for body in body_setups:
    row = {
        "bone_name": str(body.get_editor_property("bone_name")),
        "linear_damping": get_prop(body.get_editor_property("default_instance"), "linear_damping"),
        "angular_damping": get_prop(body.get_editor_property("default_instance"), "angular_damping"),
        "mass_scale": get_prop(body.get_editor_property("default_instance"), "mass_scale"),
        "sphyl_count": 0,
        "sphyls": [],
        "box_count": 0,
        "sphere_count": 0,
    }
    agg = body.get_editor_property("agg_geom")
    try:
        sphyls = agg.get_editor_property("sphyl_elems")
        row["sphyl_count"] = len(sphyls)
        for sphyl in sphyls:
            row["sphyls"].append({
                "radius": get_prop(sphyl, "radius"),
                "length": get_prop(sphyl, "length"),
                "center": vec_to_list(sphyl.get_editor_property("center")),
                "rotation": rot_to_list(sphyl.get_editor_property("rotation")),
            })
    except Exception as exc:
        row["sphyl_error"] = str(exc)
    for prop_name, out_name in (("box_elems", "box_count"), ("sphere_elems", "sphere_count")):
        try:
            row[out_name] = len(agg.get_editor_property(prop_name))
        except Exception:
            pass
    body_rows.append(row)

constraint_rows = []
for template in constraint_setups:
    ci = template.get_editor_property("default_instance")
    profile = get_prop(ci, "profile_instance")
    row = {
        "constraint_bone1": get_prop(ci, "constraint_bone1"),
        "constraint_bone2": get_prop(ci, "constraint_bone2"),
        "joint_name": get_prop(ci, "joint_name"),
        "linear_x_motion": get_prop(ci, "linear_x_motion"),
        "linear_y_motion": get_prop(ci, "linear_y_motion"),
        "linear_z_motion": get_prop(ci, "linear_z_motion"),
        "linear_limit_size": get_prop(ci, "linear_limit_size"),
        "angular_swing1_motion": get_prop(ci, "angular_swing1_motion"),
        "angular_swing2_motion": get_prop(ci, "angular_swing2_motion"),
        "angular_twist_motion": get_prop(ci, "angular_twist_motion"),
        "angular_swing1_limit": get_prop(ci, "angular_swing1_limit"),
        "angular_swing2_limit": get_prop(ci, "angular_swing2_limit"),
        "angular_twist_limit": get_prop(ci, "angular_twist_limit"),
        "disable_collision": get_prop(ci, "disable_collision"),
        "parent_dominates": get_prop(ci, "parent_dominates"),
        "profile_instance": profile,
    }
    constraint_rows.append(row)

payload = {
    "asset": ASSET_PATH,
    "body_count": len(body_rows),
    "constraint_count": len(constraint_rows),
    "bodies": body_rows,
    "constraints": constraint_rows,
}

OUTPUT_PATH.write_text(json.dumps(payload, indent=2), encoding="utf-8")
print(f"Wrote {OUTPUT_PATH}")

unreal.SystemLibrary.quit_editor()
