"""
Validate the Unreal-side FriendSlop raw humanoid skeletal asset and PhysicsAsset.

The validation is intentionally scoped to the TestRoom ragdoll/PAC spike. It
does not mutate assets.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.SystemLibrary.get_project_directory())
DEFAULT_REPORT_PATH = PROJECT_ROOT / "Reports" / "AgentReviews" / "FriendSlopUnrealRagdollImport" / "friendslop_humanoid_skeletal_validation_report.json"
DEFAULT_COMMANDLET_REPORT_PATH = PROJECT_ROOT / "Reports" / "AgentReviews" / "FriendSlopUnrealRagdollImport" / "friendslop_humanoid_physics_asset_report.json"
MESH_REF = (
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/"
    "SK_Hero_1_Chad_Male_FriendSlop.SK_Hero_1_Chad_Male_FriendSlop"
)
PHYSICS_ASSET_REF = (
    "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/"
    "PA_Hero_1_Chad_Male_FriendSlop_TestRoom.PA_Hero_1_Chad_Male_FriendSlop_TestRoom"
)
REQUIRED_PHYSICS_BODIES = [
    "pelvis",
    "spine_01",
    "spine_02",
    "spine_03",
    "head",
    "upperarm_l",
    "lowerarm_l",
    "upperarm_r",
    "lowerarm_r",
    "thigh_l",
    "calf_l",
    "foot_l",
    "thigh_r",
    "calf_r",
    "foot_r",
]
MIN_BODY_COUNT = 15
MIN_CONSTRAINT_COUNT = 12


def load_asset(ref: str):
    return unreal.EditorAssetLibrary.load_asset(ref)


def body_bone_name(body_setup: object) -> str:
    for property_name in ("bone_name", "BoneName"):
        try:
            value = body_setup.get_editor_property(property_name)
            if value:
                return str(value)
        except Exception:
            pass
    try:
        return str(body_setup.bone_name)
    except Exception:
        return ""


def material_paths(mesh: object) -> list[str]:
    paths: list[str] = []
    try:
        slots = list(mesh.get_editor_property("materials") or [])
    except Exception:
        slots = []
    for slot in slots:
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            material = None
        paths.append(material.get_path_name() if material else "")
    return paths


def mesh_bounds(mesh: object) -> dict[str, list[float] | str]:
    try:
        bounds = mesh.get_bounds()
        return {
            "origin_cm": [float(bounds.origin.x), float(bounds.origin.y), float(bounds.origin.z)],
            "extent_cm": [float(bounds.box_extent.x), float(bounds.box_extent.y), float(bounds.box_extent.z)],
            "size_cm": [
                float(bounds.box_extent.x) * 2.0,
                float(bounds.box_extent.y) * 2.0,
                float(bounds.box_extent.z) * 2.0,
            ],
        }
    except Exception as exc:
        return {"error": str(exc)}


def read_sequence_property(target: object, property_names: list[str]) -> list[object]:
    for property_name in property_names:
        try:
            values = target.get_editor_property(property_name)
            if values:
                return list(values)
        except Exception:
            pass
        try:
            values = getattr(target, property_name)
            if values:
                return list(values)
        except Exception:
            pass
    return []


def commandlet_physics_report(path: Path) -> dict[str, object] | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8-sig"))
    except Exception:
        return None


def collect_physics_asset_report(physics_asset: object | None, commandlet_report_path: Path) -> dict[str, object]:
    if not physics_asset:
        return {
            "ok": False,
            "error": "missing physics asset",
            "body_count": 0,
            "constraint_count": 0,
            "body_bones": [],
            "missing_required_body_bones": REQUIRED_PHYSICS_BODIES,
        }

    bodies = read_sequence_property(physics_asset, ["skeletal_body_setups", "skeletal_body_setups_"])
    constraints = read_sequence_property(physics_asset, ["constraint_setup", "constraint_setup_"])

    body_names = [body_bone_name(body) for body in bodies]
    if not body_names:
        report = commandlet_physics_report(commandlet_report_path)
        if report:
            body_names = [str(name) for name in report.get("body_bones", [])]
            constraints = [str(pair) for pair in report.get("constraint_pairs", [])]

    body_name_set = set(body_names)
    missing = [name for name in REQUIRED_PHYSICS_BODIES if name not in body_name_set]
    ok = len(body_names) >= MIN_BODY_COUNT and len(constraints) >= MIN_CONSTRAINT_COUNT and not missing
    return {
        "ok": ok,
        "body_count": len(body_names),
        "constraint_count": len(constraints),
        "body_bones": body_names,
        "missing_required_body_bones": missing,
        "min_body_count": MIN_BODY_COUNT,
        "min_constraint_count": MIN_CONSTRAINT_COUNT,
        "body_source": "python_reflection" if bodies else "commandlet_report",
        "commandlet_report": str(commandlet_report_path),
    }


def main() -> int:
    mesh_ref = os.environ.get("T66_FRIENDSLOP_HUMANOID_MESH_REF", MESH_REF)
    physics_asset_ref = os.environ.get("T66_FRIENDSLOP_HUMANOID_PHYSICS_ASSET_REF", PHYSICS_ASSET_REF)
    report_path = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_VALIDATION_REPORT", DEFAULT_REPORT_PATH))
    commandlet_report_path = Path(os.environ.get("T66_FRIENDSLOP_HUMANOID_PHYSICS_REPORT", DEFAULT_COMMANDLET_REPORT_PATH))
    report_path.parent.mkdir(parents=True, exist_ok=True)

    mesh = load_asset(mesh_ref)
    physics_asset = load_asset(physics_asset_ref)
    assigned_physics_asset = None
    skeleton = None
    if mesh:
        try:
            assigned_physics_asset = mesh.get_editor_property("physics_asset")
        except Exception:
            assigned_physics_asset = None
        try:
            skeleton = mesh.get_editor_property("skeleton")
        except Exception:
            skeleton = None

    physics_report = collect_physics_asset_report(physics_asset, commandlet_report_path)
    assigned_ref = assigned_physics_asset.get_path_name() if assigned_physics_asset else ""
    skeleton_ref = skeleton.get_path_name() if skeleton else ""
    mats = material_paths(mesh) if mesh else []
    errors: list[str] = []
    if not mesh:
        errors.append(f"missing skeletal mesh: {mesh_ref}")
    if not skeleton_ref:
        errors.append("skeletal mesh has no Skeleton")
    if not mats or not all(mats):
        errors.append("skeletal mesh has empty material slots")
    if not physics_asset:
        errors.append(f"missing physics asset: {physics_asset_ref}")
    if assigned_ref != (physics_asset.get_path_name() if physics_asset else ""):
        errors.append(f"skeletal mesh physics asset mismatch: assigned={assigned_ref} expected={physics_asset_ref}")
    if not physics_report.get("ok"):
        errors.append("physics asset failed body/constraint readiness gate")

    report = {
        "ok": not errors,
        "errors": errors,
        "skeletal_mesh": mesh_ref,
        "skeleton": skeleton_ref,
        "assigned_physics_asset": assigned_ref,
        "expected_physics_asset": physics_asset_ref,
        "material_paths": mats,
        "bounds": mesh_bounds(mesh) if mesh else {},
        "physics_asset": physics_report,
    }
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"[FriendSlopRawHumanoidValidate] report={report_path} ok={report['ok']}")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
    if errors:
        raise RuntimeError("; ".join(errors))
    return 0


main()
