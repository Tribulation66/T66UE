"""
Create or reload DT_CombatVFXBindings from Content/Data/CombatVFXBindings.csv.

Run from the editor or command line:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/SetupCombatVFXBindingsDataTable.py"
"""

import csv
import os

try:
    import unreal
except ImportError:
    unreal = None


DT_PATH = "/Game/Data/DT_CombatVFXBindings"
BP_GAME_INSTANCE_ASSET_PATH = "/Game/Blueprints/Core/BP_T66GameInstance"
BP_GAME_INSTANCE_CLASS_PATH = "/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C"
CSV_FIELDNAMES = [
    "---",
    "BindingID",
    "SourceType",
    "SourceID",
    "AttackCategory",
    "NiagaraSystem",
    "EffectPacketID",
    "VFXProfile",
    "bSuppressTemporaryProjectile",
    "bDevelopmentFallbackAllowed",
    "BaseVisualRadius",
    "BasePlaybackSeconds",
    "VisualScaleMultiplier",
    "Notes",
]
HERO1_AXE_AOE_BINDING_ROW = {
    "---": "Hero1Axe_AOE_Base",
    "BindingID": "Hero1Axe_AOE_Base",
    "SourceType": "WeaponBase",
    "SourceID": "Hero_1_black_aoe",
    "AttackCategory": "AOE",
    "NiagaraSystem": "/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash",
    "EffectPacketID": "Hero1AxeAOESlashMechanismPacket",
    "VFXProfile": "MeshSlashAOE",
    "bSuppressTemporaryProjectile": "True",
    "bDevelopmentFallbackAllowed": "True",
    "BaseVisualRadius": "411.4",
    "BasePlaybackSeconds": "0.46",
    "VisualScaleMultiplier": "1.0",
    "Notes": "Accepted Hero 1 AOE slash production binding; idol overlays deferred.",
}
HERO3_AOE_PLACEHOLDER_BINDING_ROW = {
    "---": "Hero3_AOE_Black_Placeholder",
    "BindingID": "Hero3_AOE_Black_Placeholder",
    "SourceType": "WeaponBase",
    "SourceID": "Hero_3_black_aoe",
    "AttackCategory": "AOE",
    "NiagaraSystem": "/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash",
    "EffectPacketID": "Hero1AxeAOESlashMechanismPacket",
    "VFXProfile": "MeshSlashAOE",
    "bSuppressTemporaryProjectile": "True",
    "bDevelopmentFallbackAllowed": "True",
    "BaseVisualRadius": "411.4",
    "BasePlaybackSeconds": "0.46",
    "VisualScaleMultiplier": "1.0",
    "Notes": "FLAGGED placeholder reuse: Hero 3 AOE temporarily reuses the accepted Hero 1 AOE slash binding until hero-specific VFX is authored.",
}
HERO2_PIERCE_BINDING_ROW = {
    "---": "Hero2_Pierce_Black_Base",
    "BindingID": "Hero2_Pierce_Black_Base",
    "SourceType": "WeaponBase",
    "SourceID": "Hero_2_black_pierce",
    "AttackCategory": "Pierce",
    "NiagaraSystem": "/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash.NS_Hero1AxePierce_MeshSlash",
    "EffectPacketID": "Hero1AxePierceMechanismPacket",
    "VFXProfile": "MeshSlashPierce",
    "bSuppressTemporaryProjectile": "True",
    "bDevelopmentFallbackAllowed": "True",
    "BaseVisualRadius": "80.0",
    "BasePlaybackSeconds": "0.3",
    "VisualScaleMultiplier": "1.0",
    "Notes": "Reclassified Pierce exemplar: Hero 2 black Pierce uses the accepted Hero 1 Pierce lane VFX until hero-specific VFX is authored.",
}
HERO4_BOUNCE_BINDING_ROW = {
    "---": "Hero4_Bounce_Black_Base",
    "BindingID": "Hero4_Bounce_Black_Base",
    "SourceType": "WeaponBase",
    "SourceID": "Hero_4_black_bounce",
    "AttackCategory": "Bounce",
    "NiagaraSystem": "/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash",
    "EffectPacketID": "Hero1AxeBounceMechanismPacket",
    "VFXProfile": "MeshSlashBounce",
    "bSuppressTemporaryProjectile": "True",
    "bDevelopmentFallbackAllowed": "True",
    "BaseVisualRadius": "80.0",
    "BasePlaybackSeconds": "0.32",
    "VisualScaleMultiplier": "1.0",
    "Notes": "Reclassified Bounce exemplar: Hero 4 black Bounce uses the accepted Hero 1 Bounce lane VFX until hero-specific VFX is authored.",
}
HERO5_DOT_BINDING_ROW = {
    "---": "Hero5_DOT_Black_Base",
    "BindingID": "Hero5_DOT_Black_Base",
    "SourceType": "WeaponBase",
    "SourceID": "Hero_5_black_dot",
    "AttackCategory": "DOT",
    "NiagaraSystem": "/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash.NS_Hero1AxeDOT_MeshSlash",
    "EffectPacketID": "Hero1AxeDOTMechanismPacket",
    "VFXProfile": "MeshSlashDOT",
    "bSuppressTemporaryProjectile": "True",
    "bDevelopmentFallbackAllowed": "True",
    "BaseVisualRadius": "80.0",
    "BasePlaybackSeconds": "0.6",
    "VisualScaleMultiplier": "1.0",
    "Notes": "Reclassified DOT exemplar: Hero 5 black DOT uses the accepted Hero 1 DOT lane VFX until hero-specific VFX is authored.",
}

RARITY_SUFFIXES = ("black", "red", "yellow", "white")


def _expand_weapon_base_binding_for_rarities(base_row, row_id_prefix, source_id_prefix, category_suffix, notes_template):
    rows = []
    for rarity_suffix in RARITY_SUFFIXES:
        row = dict(base_row)
        title_suffix = rarity_suffix.capitalize()
        row_id = row_id_prefix if rarity_suffix == "black" else f"{row_id_prefix}_{title_suffix}"
        row["---"] = row_id
        row["BindingID"] = row_id
        row["SourceID"] = f"{source_id_prefix}_{rarity_suffix}_{category_suffix}"
        row["Notes"] = notes_template.format(rarity=title_suffix)
        rows.append(row)
    return rows


ENFORCED_BINDING_ROWS = (
    _expand_weapon_base_binding_for_rarities(
        HERO1_AXE_AOE_BINDING_ROW,
        "Hero1Axe_AOE_Base",
        "Hero_1",
        "aoe",
        "Accepted Hero 1 {rarity} AOE slash production binding; idol overlays deferred.",
    )
    + _expand_weapon_base_binding_for_rarities(
        HERO3_AOE_PLACEHOLDER_BINDING_ROW,
        "Hero3_AOE_Black_Placeholder",
        "Hero_3",
        "aoe",
        "FLAGGED placeholder reuse: Hero 3 {rarity} AOE temporarily reuses the accepted Hero 1 AOE slash binding until hero-specific VFX is authored.",
    )
    + _expand_weapon_base_binding_for_rarities(
        HERO2_PIERCE_BINDING_ROW,
        "Hero2_Pierce_Black_Base",
        "Hero_2",
        "pierce",
        "Reclassified Pierce exemplar: Hero 2 {rarity} Pierce uses the accepted Hero 1 Pierce lane VFX until hero-specific VFX is authored.",
    )
    + _expand_weapon_base_binding_for_rarities(
        HERO4_BOUNCE_BINDING_ROW,
        "Hero4_Bounce_Black_Base",
        "Hero_4",
        "bounce",
        "Reclassified Bounce exemplar: Hero 4 {rarity} Bounce uses the accepted Hero 1 Bounce lane VFX until hero-specific VFX is authored.",
    )
    + _expand_weapon_base_binding_for_rarities(
        HERO5_DOT_BINDING_ROW,
        "Hero5_DOT_Black_Base",
        "Hero_5",
        "dot",
        "Reclassified DOT exemplar: Hero 5 {rarity} DOT uses the accepted Hero 1 DOT lane VFX until hero-specific VFX is authored.",
    )
)


def resolve_row_struct():
    for name in ("T66CombatVFXBindingData", "CombatVFXBindingData", "FT66CombatVFXBindingData"):
        struct_type = getattr(unreal, name, None)
        if struct_type is None:
            continue
        if hasattr(struct_type, "static_struct"):
            return struct_type.static_struct()
        return struct_type

    unreal.log_error("Could not resolve FT66CombatVFXBindingData in Python.")
    return None


def load_or_create_datatable(row_struct):
    if unreal.EditorAssetLibrary.does_asset_exist(DT_PATH):
        data_table = unreal.EditorAssetLibrary.load_asset(DT_PATH)
        if data_table:
            return data_table

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    package_path, asset_name = DT_PATH.rsplit("/", 1)
    data_table = asset_tools.create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not data_table:
        unreal.log_error(f"Failed to create DataTable asset at {DT_PATH}")
        return None

    unreal.log(f"Created DataTable asset at {DT_PATH}")
    return data_table


def assign_game_instance_data_table(data_table):
    bp_class = unreal.load_object(None, BP_GAME_INSTANCE_CLASS_PATH)
    if not bp_class:
        unreal.log_warning(f"Could not load BP_T66GameInstance class at {BP_GAME_INSTANCE_CLASS_PATH}")
        return

    cdo = unreal.get_default_object(bp_class)
    if not cdo:
        unreal.log_warning("Could not resolve BP_T66GameInstance CDO.")
        return

    cdo.set_editor_property("CombatVFXBindingsDataTable", data_table)

    if unreal.EditorAssetLibrary.save_asset(BP_GAME_INSTANCE_ASSET_PATH):
        unreal.log("Assigned CombatVFXBindingsDataTable on BP_T66GameInstance.")
    else:
        unreal.log_warning("Failed to save BP_T66GameInstance after assigning CombatVFXBindingsDataTable.")


def ensure_combat_vfx_bindings_csv(csv_path):
    rows = []
    if os.path.exists(csv_path):
        with open(csv_path, newline="", encoding="utf-8-sig") as csv_file:
            reader = csv.DictReader(csv_file)
            for row in reader:
                normalized = {field: row.get(field, "") for field in CSV_FIELDNAMES}
                rows.append(normalized)

    for enforced_row in ENFORCED_BINDING_ROWS:
        row_index = next(
            (
                index
                for index, row in enumerate(rows)
                if row.get("---") == enforced_row["---"]
                or row.get("BindingID") == enforced_row["BindingID"]
            ),
            None,
        )
        if row_index is None:
            rows.append(dict(enforced_row))
        else:
            rows[row_index] = dict(enforced_row)

    os.makedirs(os.path.dirname(csv_path), exist_ok=True)
    with open(csv_path, "w", newline="", encoding="utf-8") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=CSV_FIELDNAMES, quoting=csv.QUOTE_ALL)
        writer.writeheader()
        writer.writerows(rows)


def main():
    if unreal is None:
        raise RuntimeError("Unreal Python module is required to create or reload DT_CombatVFXBindings.")

    unreal.log("=== SetupCombatVFXBindingsDataTable ===")

    project_dir = unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    csv_path = os.path.normpath(os.path.join(project_dir, "Content", "Data", "CombatVFXBindings.csv"))
    ensure_combat_vfx_bindings_csv(csv_path)
    unreal.log(f"CombatVFXBindings.csv enforced at {csv_path}")

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, csv_path)
    if not success:
        unreal.log_error("Failed to fill DT_CombatVFXBindings from CSV.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    assign_game_instance_data_table(data_table)

    unreal.log(f"DT_CombatVFXBindings reloaded from {csv_path}")
    unreal.log("=== SetupCombatVFXBindingsDataTable DONE ===")


if __name__ == "__main__":
    if unreal is None:
        print("Unreal Python module is required to create or reload DT_CombatVFXBindings.")
    else:
        main()
