"""
Verify the current import/gameplay batch in-editor.

Checks:
  - Imported prop / interactable / bag / goblin assets exist
  - Imported materials are parented to the expected Unlit masters
  - DataTables contain the expected new rows and removed Leprechaun row
  - Fountain interactable defaults point at the imported fountain mesh

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

STATIC_MESH_CHECKS = [
    ("Prop Tree2", "/Game/World/Props/Tree2.Tree2", GLB_UNLIT_PARENTS),
    ("Prop Rock", "/Game/World/Props/Rock.Rock", GLB_UNLIT_PARENTS),
    ("Prop Grass", "/Game/World/Props/Grass.Grass", GLB_UNLIT_PARENTS),
    ("Prop Bush", "/Game/World/Props/Bush.Bush", GLB_UNLIT_PARENTS),
    ("Prop Boulder", "/Game/World/Props/Boulder.Boulder", GLB_UNLIT_PARENTS),
    ("Fountain Mesh", "/Game/World/Interactables/Fountain/Fountain.Fountain", GLB_UNLIT_PARENTS),
    ("LootBag Black", "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black", GLB_UNLIT_PARENTS),
    ("LootBag Red", "/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red", GLB_UNLIT_PARENTS),
    ("LootBag Yellow", "/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow", GLB_UNLIT_PARENTS),
    ("LootBag White", "/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White", GLB_UNLIT_PARENTS),
]

SKELETAL_MESH_CHECKS = [
    (
        "Goblin Black",
        "/Game/Characters/Enemies/GoblinThief/Black/BlackGoblinRun.BlackGoblinRun",
        "/Game/Characters/Enemies/GoblinThief/Black/BlackGoblinRun_Anim.BlackGoblinRun_Anim",
    ),
    (
        "Goblin Red",
        "/Game/Characters/Enemies/GoblinThief/Red/RedGoblinRun.RedGoblinRun",
        "/Game/Characters/Enemies/GoblinThief/Red/RedGoblinRun_Anim.RedGoblinRun_Anim",
    ),
    (
        "Goblin Yellow",
        "/Game/Characters/Enemies/GoblinThief/Yellow/YellowGoblinRun.YellowGoblinRun",
        "/Game/Characters/Enemies/GoblinThief/Yellow/YellowGoblinRun_Anim.YellowGoblinRun_Anim",
    ),
    (
        "Goblin White",
        "/Game/Characters/Enemies/GoblinThief/White/WhiteGoblinRun.WhiteGoblinRun",
        "/Game/Characters/Enemies/GoblinThief/White/WhiteGoblinRun_Anim.WhiteGoblinRun_Anim",
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
        for expected in ("Tree2", "Rock", "Grass", "Bush", "Boulder"):
            v.add(f"DT_Props row {expected}", expected in rows, f"rows={len(rows)}")

    if dt_items:
        rows = _get_row_names(dt_items)
        v.add("DT_Items removed Item_Leprechaun", "Item_Leprechaun" not in rows, f"rows={len(rows)}")
        v.add("DT_Items removed Item_Goblin", "Item_Goblin" not in rows, f"rows={len(rows)}")
        v.add("DT_Items row Item_LootCrate", "Item_LootCrate" in rows, f"rows={len(rows)}")

    if dt_visuals:
        rows = _get_row_names(dt_visuals)
        for expected in ("GoblinThief_Black", "GoblinThief_Red", "GoblinThief_Yellow", "GoblinThief_White"):
            v.add(f"DT_CharacterVisuals row {expected}", expected in rows, f"rows={len(rows)}")


def _check_fountain_defaults(v):
    cls = unreal.load_class(None, "/Script/T66.T66TreeOfLifeInteractable")
    v.add("Fountain interactable class", bool(cls), "/Script/T66.T66TreeOfLifeInteractable")
    if not cls:
        return

    try:
        cdo = unreal.get_default_object(cls)
    except Exception as exc:
        v.add("Fountain interactable CDO", False, exc)
        return

    single_mesh = _get_asset_path_string(cdo.get_editor_property("single_mesh"))
    v.add(
        "Fountain default single mesh",
        "/Game/World/Interactables/Fountain/Fountain.Fountain" in single_mesh,
        single_mesh,
    )

    rarity_meshes = cdo.get_editor_property("rarity_meshes")
    try:
        rarity_count = len(rarity_meshes)
    except Exception:
        rarity_count = -1
    v.add("Fountain default rarity mesh map empty", rarity_count == 0, f"count={rarity_count}")


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
        ("Tree2", "/Game/World/Props/Tree2.Tree2"),
        ("Rock", "/Game/World/Props/Rock.Rock"),
        ("BlackBag", "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black"),
        ("RedBag", "/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red"),
        ("YellowBag", "/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow"),
        ("WhiteBag", "/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White"),
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
        ("/Game/Materials/Retro/M_Character_Unlit_RetroGeometry.M_Character_Unlit_RetroGeometry", "used_with_skeletal_mesh", True),
        ("/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry.M_FBX_Unlit_RetroGeometry", "used_with_skeletal_mesh", True),
        ("/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry.M_GLB_Unlit_RetroGeometry", "used_with_instanced_static_meshes", True),
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
    _check_fountain_defaults(verification)
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
