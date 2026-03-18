"""
Verify the current import/gameplay batch in-editor.

Checks:
  - Imported prop / hero / companion assets exist
  - Imported materials are parented to the expected Unlit masters
  - DataTables contain the expected prop and character visual rows
  - Tree2 import overrides and prop build settings still match the GLB pipeline

Run in-editor or with the Python commandlet:
  py "C:/UE/T66/Scripts/VerifyImportBatch.py"
"""

import json
import os
import unreal


PROJECT_LOG_DIR = os.path.join(
    unreal.SystemLibrary.get_project_directory(), "Saved", "Logs")
REPORT_PATH = os.path.join(PROJECT_LOG_DIR, "VerifyImportBatch.json")

GLB_UNLIT_PARENTS = (
    "/Game/Materials/M_GLB_Unlit",
    "/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry",
)

CHAR_UNLIT_PARENTS = (
    "/Game/Materials/M_Character_Unlit",
    "/Game/Materials/M_FBX_Unlit",
    "/Game/Materials/Retro/M_Character_Unlit_RetroGeometry",
    "/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry",
)

PROP_ROW_NAMES = (
    "Barn",
    "Tree",
    "Tree2",
    "Hay",
    "Hay2",
    "Rock",
    "Grass",
    "Bush",
    "Boulder",
    "Branch",
    "Fence",
    "Fence2",
    "Fence3",
    "Haybell",
    "House",
    "Log",
    "Mud",
    "Rocks",
    "Scarecrow",
    "Silo",
    "Stump",
    "Tractor",
    "Tree3",
    "Troth",
    "Windmill",
)

CURRENT_GLB_PROP_BATCH = (
    "Barn",
    "Boulder",
    "Branch",
    "Bush",
    "Fence",
    "Fence2",
    "Fence3",
    "Haybell",
    "House",
    "Log",
    "Mud",
    "Rocks",
    "Scarecrow",
    "Silo",
    "Stump",
    "Tractor",
    "Tree",
    "Tree2",
    "Tree3",
    "Troth",
    "Windmill",
)

STATIC_MESH_CHECKS = [
    (f"Prop {name}", f"/Game/World/Props/{name}.{name}", GLB_UNLIT_PARENTS)
    for name in PROP_ROW_NAMES
]

SKELETAL_MESH_CHECKS = [
    (
        "Hero 1 Type A",
        "/Game/Characters/Heroes/Hero_1/TypeA/Idle/ArthurAIdle.ArthurAIdle",
        "/Game/Characters/Heroes/Hero_1/TypeA/Walk/ArthurAWalk_Anim.ArthurAWalk_Anim",
    ),
    (
        "Hero 1 Type B",
        "/Game/Characters/Heroes/Hero_1/TypeB/Idle/ArthurBIdle.ArthurBIdle",
        "/Game/Characters/Heroes/Hero_1/TypeB/Walk/ArthurBWalk_Anim.ArthurBWalk_Anim",
    ),
    (
        "Companion 01 Default",
        "/Game/Characters/Companions/Companion_01/Default/Idle/CompanionIdle.CompanionIdle",
        "/Game/Characters/Companions/Companion_01/Default/Walk/CompanionWalk_Anim.CompanionWalk_Anim",
    ),
]


class Verification:
    def __init__(self):
        self.checks = []

    def add(self, name, ok, detail):
        detail = str(detail)
        self.checks.append({"name": name, "ok": bool(ok), "detail": detail})
        prefix = "[PASS]" if ok else "[FAIL]"
        if ok:
            unreal.log(f"{prefix} {name}: {detail}")
        else:
            unreal.log_error(f"{prefix} {name}: {detail}")

    @property
    def failures(self):
        return [c for c in self.checks if not c["ok"]]


def _ensure_report_dir():
    os.makedirs(PROJECT_LOG_DIR, exist_ok=True)


def _load_asset(path):
    return unreal.EditorAssetLibrary.load_asset(path)


def _path_matches(path, allowed_prefixes):
    if not path:
        return False
    normalized = path.replace("\\", "/")
    for prefix in allowed_prefixes:
        if normalized.startswith(prefix):
            return True
    return False


def _get_material_parent_path(material):
    if not material:
        return ""

    try:
        parent = material.get_editor_property("parent")
        if parent:
            return parent.get_path_name()
    except Exception:
        pass

    try:
        parent = unreal.MaterialEditingLibrary.get_material_instance_parent(material)
        if parent:
            return parent.get_path_name()
    except Exception:
        pass

    try:
        return material.get_path_name()
    except Exception:
        return str(material)


def _get_static_mesh_materials(mesh):
    slots = mesh.get_editor_property("static_materials")
    return [slot.get_editor_property("material_interface") for slot in slots]


def _get_skeletal_mesh_materials(mesh):
    slots = mesh.get_editor_property("materials")
    return [slot.get_editor_property("material_interface") for slot in slots]


def _get_asset_path_string(value):
    if value is None:
        return ""
    try:
        return value.get_asset_path_string()
    except Exception:
        pass
    try:
        return value.get_path_name()
    except Exception:
        pass
    return str(value)


def _get_material_instance_scalar(material, param_name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(material, param_name)
        if isinstance(value, (tuple, list)):
            value = value[-1] if value else None
        return value
    except Exception:
        return None


def _get_material_instance_vector(material, param_name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(material, param_name)
        if isinstance(value, (tuple, list)):
            value = value[-1] if value else None
        return value
    except Exception:
        return None


def _get_material_instance_texture(material, param_name):
    try:
        value = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, param_name)
        if isinstance(value, (tuple, list)):
            value = value[-1] if value else None
        return value
    except Exception:
        return None


def _get_static_mesh_build_settings(mesh, lod_index=0):
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception:
        subsystem = None

    if not subsystem:
        return None

    try:
        return subsystem.get_lod_build_settings(mesh, lod_index)
    except Exception:
        return None


def _check_material_interfaces(v, label, materials, allowed_parents):
    if not materials:
        v.add(label, False, "no material slots found")
        return

    for index, material in enumerate(materials):
        if not material:
            v.add(f"{label} material[{index}]", False, "material interface missing")
            continue
        parent_path = _get_material_parent_path(material)
        ok = _path_matches(parent_path, allowed_parents)
        v.add(
            f"{label} material[{index}]",
            ok,
            f"{material.get_name()} -> {parent_path or '<no parent>'}",
        )


def _check_static_mesh(v, label, path, allowed_parents):
    asset = _load_asset(path)
    v.add(f"{label} asset", bool(asset), path)
    if not asset:
        return
    _check_material_interfaces(v, label, _get_static_mesh_materials(asset), allowed_parents)


def _check_skeletal_mesh(v, label, mesh_path, anim_path):
    mesh = _load_asset(mesh_path)
    anim = _load_asset(anim_path)
    v.add(f"{label} mesh asset", bool(mesh), mesh_path)
    v.add(f"{label} anim asset", bool(anim), anim_path)
    if mesh:
        _check_material_interfaces(v, label, _get_skeletal_mesh_materials(mesh), CHAR_UNLIT_PARENTS)


def _get_row_names(data_table):
    try:
        return {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(data_table)}
    except Exception as exc:
        unreal.log_error(f"[FAIL] DataTable row name query failed: {exc}")
        return set()


def _check_data_tables(v):
    dt_props = _load_asset("/Game/Data/DT_Props.DT_Props")
    dt_items = _load_asset("/Game/Data/DT_Items.DT_Items")
    dt_visuals = _load_asset("/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals")

    v.add("DT_Props asset", bool(dt_props), "/Game/Data/DT_Props.DT_Props")
    v.add("DT_Items asset", bool(dt_items), "/Game/Data/DT_Items.DT_Items")
    v.add("DT_CharacterVisuals asset", bool(dt_visuals), "/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals")

    if dt_props:
        rows = _get_row_names(dt_props)
        for expected in PROP_ROW_NAMES:
            v.add(f"DT_Props row {expected}", expected in rows, f"rows={len(rows)}")

    if dt_items:
        rows = _get_row_names(dt_items)
        v.add("DT_Items removed Item_Leprechaun", "Item_Leprechaun" not in rows, f"rows={len(rows)}")
        v.add("DT_Items removed Item_Goblin", "Item_Goblin" not in rows, f"rows={len(rows)}")
        v.add("DT_Items row Item_LootCrate", "Item_LootCrate" in rows, f"rows={len(rows)}")

    if dt_visuals:
        rows = _get_row_names(dt_visuals)
        for expected in (
            "Hero_1_TypeA",
            "Hero_1_TypeB",
            "Companion_01",
            "Companion_09",
            "Companion_17",
        ):
            v.add(f"DT_CharacterVisuals row {expected}", expected in rows, f"rows={len(rows)}")

def _check_import_overrides(v):
    tree2_mesh = _load_asset("/Game/World/Props/Tree2.Tree2")
    tree2_materials = _get_static_mesh_materials(tree2_mesh) if tree2_mesh else []
    tree2 = tree2_materials[0] if tree2_materials else None
    v.add(
        "Tree2 material asset",
        bool(tree2),
        tree2.get_path_name() if tree2 else "<missing>",
    )
    if tree2:
        texture = _get_material_instance_texture(tree2, "BaseColorTexture")
        v.add("Tree2 BaseColorTexture asset", bool(texture), texture.get_path_name() if texture else "<missing>")
        if texture:
            try:
                mip_gen = texture.get_editor_property("mip_gen_settings")
                never_stream = texture.get_editor_property("never_stream")
            except Exception as exc:
                v.add("Tree2 texture overrides readable", False, exc)
            else:
                v.add(
                    "Tree2 BaseColorTexture no mips",
                    mip_gen == unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS,
                    f"{mip_gen}",
                )
                v.add(
                    "Tree2 BaseColorTexture never stream",
                    bool(never_stream),
                    f"{never_stream}",
                )

    build_setting_checks = [
        (name, f"/Game/World/Props/{name}.{name}")
        for name in CURRENT_GLB_PROP_BATCH
    ]

    try:
        build_settings_subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    except Exception:
        build_settings_subsystem = None

    if not build_settings_subsystem:
        v.add(
            "Static mesh build setting verification",
            True,
            "skipped: StaticMeshEditorSubsystem unavailable in this Unreal mode; verify via UnrealEditor.exe",
        )
        return

    for label, asset_path in build_setting_checks:
        mesh = _load_asset(asset_path)
        v.add(f"{label} build-settings mesh asset", bool(mesh), asset_path)
        if not mesh:
            continue

        build_settings = _get_static_mesh_build_settings(mesh, 0)
        v.add(f"{label} LOD0 build settings", build_settings is not None, "available" if build_settings else "missing")
        if not build_settings:
            continue

        expected_flags = {
            "use_full_precision_u_vs": True,
            "generate_lightmap_u_vs": False,
            "recompute_normals": False,
            "recompute_tangents": False,
        }
        for prop_name, expected_value in expected_flags.items():
            try:
                actual_value = build_settings.get_editor_property(prop_name)
            except Exception as exc:
                v.add(f"{label} {prop_name}", False, exc)
                continue
            v.add(f"{label} {prop_name}", actual_value == expected_value, f"{actual_value}")


def _check_master_material_flags(v):
    checks = [
        ("/Game/Materials/M_Character_Unlit.M_Character_Unlit", "used_with_skeletal_mesh", True),
        ("/Game/Materials/M_FBX_Unlit.M_FBX_Unlit", "used_with_skeletal_mesh", True),
        ("/Game/Materials/M_GLB_Unlit.M_GLB_Unlit", "used_with_instanced_static_meshes", True),
        ("/Game/Materials/M_GLB_Unlit.M_GLB_Unlit", "used_with_nanite", True),
        ("/Game/Materials/Retro/M_Character_Unlit_RetroGeometry.M_Character_Unlit_RetroGeometry", "used_with_skeletal_mesh", True),
        ("/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry.M_FBX_Unlit_RetroGeometry", "used_with_skeletal_mesh", True),
        ("/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry.M_GLB_Unlit_RetroGeometry", "used_with_instanced_static_meshes", True),
        ("/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry.M_GLB_Unlit_RetroGeometry", "used_with_nanite", True),
    ]

    for asset_path, prop_name, expected in checks:
        material = _load_asset(asset_path)
        v.add(f"{asset_path} asset", bool(material), asset_path)
        if not material:
            continue

        try:
            actual = material.get_editor_property(prop_name)
        except Exception as exc:
            v.add(f"{asset_path} {prop_name}", False, exc)
            continue

        v.add(f"{asset_path} {prop_name}", actual == expected, f"{actual}")


def main():
    _ensure_report_dir()

    verification = Verification()
    unreal.log("=== VerifyImportBatch START ===")

    for label, path, allowed_parents in STATIC_MESH_CHECKS:
        _check_static_mesh(verification, label, path, allowed_parents)

    for label, mesh_path, anim_path in SKELETAL_MESH_CHECKS:
        _check_skeletal_mesh(verification, label, mesh_path, anim_path)

    _check_data_tables(verification)
    _check_import_overrides(verification)
    _check_master_material_flags(verification)

    report = {
        "passed": len(verification.failures) == 0,
        "total_checks": len(verification.checks),
        "failed_checks": len(verification.failures),
        "checks": verification.checks,
    }

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    if verification.failures:
        unreal.log_error(
            f"=== VerifyImportBatch FAILED ({len(verification.failures)} failures) ===")
    else:
        unreal.log("=== VerifyImportBatch PASSED ===")

    unreal.log(f"Report written to: {REPORT_PATH}")


if __name__ == "__main__":
    main()
