"""
Build the first required inventory manifest for the world/NPC interactables
retro batch, then request editor shutdown.

This script is intentionally read-heavy: it audits live data rows, runtime mesh
references, previous generation-run artifacts, and Unreal asset existence before
any new source/Trellis/Quad Retro work is attempted.
"""

import csv
import glob
import json
import os
import sys
from datetime import datetime
from pathlib import Path

import unreal


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.append(str(SCRIPT_DIR))

import ImportStaticMeshes


OUTPUT_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Interactables" / "WorldNpcInteractablesRetroBatch01"
REPORTS_DIR = OUTPUT_ROOT / "Reports"
NOTES_DIR = OUTPUT_ROOT / "Notes"

ARCADE_DATA = PROJECT_ROOT / "Content" / "Data" / "ArcadeInteractables.json"
PROPS_DATA = PROJECT_ROOT / "Content" / "Data" / "Props.csv"
HOUSE_NPCS_DATA = PROJECT_ROOT / "Content" / "Data" / "HouseNPCs.csv"
CHARACTER_VISUALS_DATA = PROJECT_ROOT / "Content" / "Data" / "CharacterVisuals.csv"

PREVIOUS_INTERACTABLE_RUN = PROJECT_ROOT / "Model Generation" / "Runs" / "Interactables" / "ArcadeReplacementBatch01"
PREVIOUS_ENV_RUN = PROJECT_ROOT / "Model Generation" / "Runs" / "Environment" / "CoherentThemeKit01"

EXPECTED_ENV_PARENT = "/Game/Materials/M_Environment_Unlit"


def _project_rel(path):
    if not path:
        return ""
    try:
        return str(Path(path).resolve().relative_to(PROJECT_ROOT)).replace("\\", "/")
    except Exception:
        return str(path).replace("\\", "/")


def _package_path(asset_ref):
    if not asset_ref:
        return ""
    path = str(asset_ref).strip()
    if not path:
        return ""
    if "." in path:
        left, right = path.rsplit(".", 1)
        if "/" in left and right:
            return left
    return path


def _asset_name(asset_path):
    package = _package_path(asset_path)
    return package.rsplit("/", 1)[-1] if package else ""


def _object_ref(package_path):
    if not package_path:
        return ""
    return f"{package_path}.{package_path.rsplit('/', 1)[-1]}"


def _load(asset_ref):
    package = _package_path(asset_ref)
    if not package:
        return None
    try:
        return unreal.EditorAssetLibrary.load_asset(package)
    except Exception:
        return None


def _asset_exists(asset_ref):
    package = _package_path(asset_ref)
    if not package:
        return False
    try:
        if unreal.EditorAssetLibrary.does_asset_exist(package):
            return True
    except Exception:
        pass
    return _load(package) is not None


def _asset_class_name(asset):
    if not asset:
        return ""
    try:
        return asset.get_class().get_name()
    except Exception:
        return type(asset).__name__


def _material_parent_path(material):
    if not material:
        return ""
    try:
        parent = material.get_editor_property("parent")
    except Exception:
        parent = None
    if not parent:
        return ""
    try:
        return parent.get_path_name().split(".", 1)[0]
    except Exception:
        return ""


def _first_static_mesh_material_parent(asset_ref):
    mesh = _load(asset_ref)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        return ""
    try:
        material = mesh.get_material(0)
    except Exception:
        material = None
    return _material_parent_path(material)


def _read_json(path):
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def _read_csv(path):
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def _read_previous_interactable_entries():
    manifest_path = PREVIOUS_INTERACTABLE_RUN / "batch_manifest.json"
    if not manifest_path.exists():
        return {}
    data = _read_json(manifest_path)
    by_id = {}
    for entry in data.get("entries", []):
        target_id = entry.get("target_id")
        if target_id:
            by_id[target_id] = entry
        row_id = entry.get("current_data_row")
        if row_id:
            by_id[row_id] = entry
    return by_id


def _previous_interactable_artifacts(entry):
    if not entry:
        return "", "", "", ""
    source_image = entry.get("source_image") or ""
    raw_glb = entry.get("trellis_output") or ""
    prompt = entry.get("prompt") or ""

    def full(rel):
        if not rel:
            return ""
        path = PREVIOUS_INTERACTABLE_RUN / rel.replace("/", os.sep)
        return _project_rel(path) if path.exists() else _project_rel(path)

    source_full = full(source_image)
    raw_full = full(raw_glb)
    prompt_full = full(prompt)
    quad = _find_existing_quad_glb(entry.get("target_id") or "")
    return source_full, raw_full, quad, prompt_full


def _find_existing_quad_glb(row_id):
    if not row_id:
        return ""
    patterns = [
        str(PROJECT_ROOT / "Model Generation" / "Runs" / "**" / f"*{row_id}*QuadRetro*.glb"),
        str(PROJECT_ROOT / "Model Generation" / "Runs" / "**" / f"*{row_id}*_QuadRetro.glb"),
    ]
    for pattern in patterns:
        matches = sorted(glob.glob(pattern, recursive=True))
        if matches:
            return _project_rel(matches[0])
    return ""


def _find_env_artifact(module_id, kind):
    if not module_id:
        return ""
    if kind == "source":
        candidates = [
            PREVIOUS_ENV_RUN / "Inputs" / "approved_seed_images" / f"{module_id}.png",
            PREVIOUS_ENV_RUN / "Inputs" / "floor_slab_seed_images" / f"{module_id}_FloorFix01.png",
        ]
    elif kind == "raw":
        candidates = sorted(PREVIOUS_ENV_RUN.glob(f"Raw/Trellis/{module_id}*_Trellis2.glb"))
    else:
        candidates = []
    for candidate in candidates:
        if candidate.exists():
            return _project_rel(candidate)
    return ""


def _target_path(category, row_id, filename, ext):
    return _project_rel(OUTPUT_ROOT / category / row_id / f"{filename}{ext}")


def _source_target(category, row_id):
    return _project_rel(OUTPUT_ROOT / "Inputs" / "SourceImages" / category / f"{row_id}.png")


def _prompt_target(category, row_id):
    return _project_rel(OUTPUT_ROOT / "Inputs" / "Prompts" / category / f"{row_id}.prompt.txt")


def _qa_target(kind, category, row_id):
    return _project_rel(OUTPUT_ROOT / "QA" / kind / category / f"{row_id}_front.png")


def _entry(
    row_id,
    source_table,
    category,
    display_name,
    runtime_class,
    current_mesh_path,
    current_asset_quality,
    existing_generation_run="",
    existing_source_image="",
    existing_raw_trellis_glb="",
    existing_quad_retro_glb="",
    needs_source_image=False,
    needs_trellis=False,
    needs_quad_retro=False,
    needs_unreal_import=False,
    needs_data_update=False,
    import_target_path="",
    data_update_target="",
    prompt_used="",
    source_image="",
    raw_trellis_glb="",
    quad_retro_glb="",
    trellis_front_render="",
    quad_retro_front_render="",
    unreal_asset_path="",
    status="",
    notes="",
):
    current_asset_exists = _asset_exists(current_mesh_path)
    if not unreal_asset_path:
        unreal_asset_path = _package_path(current_mesh_path)
    if not import_target_path and unreal_asset_path:
        import_target_path = unreal_asset_path

    if current_mesh_path and current_asset_exists:
        asset = _load(current_mesh_path)
        class_name = _asset_class_name(asset)
        parent = _first_static_mesh_material_parent(current_mesh_path)
        if parent:
            current_asset_quality = f"{current_asset_quality}; asset_class={class_name}; material_parent={parent}"
        else:
            current_asset_quality = f"{current_asset_quality}; asset_class={class_name}"

    return {
        "row_id": row_id,
        "source_table": source_table,
        "category": category,
        "display_name": display_name,
        "runtime_class": runtime_class,
        "current_mesh_path": current_mesh_path,
        "current_asset_exists": current_asset_exists,
        "current_asset_quality": current_asset_quality,
        "existing_generation_run": existing_generation_run,
        "existing_source_image": existing_source_image,
        "existing_raw_trellis_glb": existing_raw_trellis_glb,
        "existing_quad_retro_glb": existing_quad_retro_glb,
        "needs_source_image": needs_source_image,
        "needs_trellis": needs_trellis,
        "needs_quad_retro": needs_quad_retro,
        "needs_unreal_import": needs_unreal_import,
        "needs_data_update": needs_data_update,
        "import_target_path": import_target_path,
        "data_update_target": data_update_target,
        "prompt_used": prompt_used,
        "source_image": source_image,
        "raw_trellis_glb": raw_trellis_glb,
        "quad_retro_glb": quad_retro_glb,
        "trellis_front_render": trellis_front_render,
        "quad_retro_front_render": quad_retro_front_render,
        "unreal_asset_path": unreal_asset_path,
        "status": status,
        "notes": notes,
    }


def _archetype_prompt(display_name, category, note):
    return (
        f"Create one centered, full-object source image for a {display_name} {category}. "
        "Plain background, no text, no UI, no scene, no floor, no cropped parts. "
        "Use a readable chunky silhouette and simple material breaks that survive low-poly Quad Retro treatment. "
        f"{note}".strip()
    )


def _build_arcade_entries(previous_entries):
    entries = []
    arcade_rows = _read_json(ARCADE_DATA)
    for row in arcade_rows:
        row_id = row.get("Name", "")
        data = row.get("ArcadeData", {})
        display_name = data.get("DisplayName", row_id)
        current_mesh = data.get("DisplayMesh", "")
        previous = previous_entries.get(row_id)
        if row_id == "Arcade_Machine":
            previous = previous_entries.get("ArcadeMachine") or previous
        source_existing, raw_existing, quad_existing, prompt_existing = _previous_interactable_artifacts(previous)
        runtime_class = "AT66ArcadeTruckInteractable" if row_id == "Vehicle" else "AT66ArcadeMachineInteractable" if row_id == "Arcade_Machine" else "AT66ArcadeInteractableBase"
        asset_name = "ArcadeMachine" if row_id == "Arcade_Machine" else row_id
        import_target = "/Game/World/Interactables/ArcadeMachine/ArcadeMachine" if row_id == "Arcade_Machine" else f"/Game/World/Interactables/Arcade/{row_id}/{row_id}"

        has_mesh = bool(current_mesh)
        has_previous_raw = bool(raw_existing)
        needs_source = not source_existing
        needs_trellis = not has_previous_raw and (not has_mesh or row_id != "Arcade_Machine")
        needs_quad = not quad_existing
        needs_import = needs_quad or not _asset_exists(current_mesh)
        needs_data_update = not has_mesh or row_id == "Vehicle"

        if row_id == "Arcade_Machine":
            quality = "existing_trellis_import_no_quad_retro_glb"
            status = "existing_asset_needs_quad_retro"
            notes = "Generic cabinet row is the guaranteed world arcade spawn source and already has a generated/imported Batch01 mesh."
            needs_trellis = False
            needs_source = False
            needs_data_update = False
            import_target = "/Game/World/Interactables/ArcadeMachine/ArcadeMachine"
        elif row_id == "Vehicle":
            quality = "missing_display_mesh_after_props_retirement"
            status = "missing_model"
            notes = "Vehicle formerly reused /Game/World/Props/Tractor; props are retired, so this needs a new non-prop interactable mesh path."
        else:
            quality = "missing_display_mesh_uses_arcade_fallback_if_spawned_directly"
            status = "missing_display_mesh"
            notes = "Arcade game row is live data for popup arcade selection but has no DisplayMesh field; generate only if direct themed world machines are desired."

        if row_id == "Vehicle":
            prompt = _archetype_prompt(display_name, "pilotable vehicle interactable", "Make it a chunky compact mower/tractor-like arcade vehicle, not a generic arcade cabinet.")
        else:
            prompt = _archetype_prompt(display_name, "arcade interactable", "Make it an arcade cabinet or compact arcade station themed to the row ID.")
        entries.append(_entry(
            row_id=row_id,
            source_table="Content/Data/ArcadeInteractables.json",
            category="Interactables",
            display_name=display_name,
            runtime_class=runtime_class,
            current_mesh_path=current_mesh,
            current_asset_quality=quality,
            existing_generation_run=_project_rel(PREVIOUS_INTERACTABLE_RUN) if previous else "",
            existing_source_image=source_existing,
            existing_raw_trellis_glb=raw_existing,
            existing_quad_retro_glb=quad_existing,
            needs_source_image=needs_source,
            needs_trellis=needs_trellis,
            needs_quad_retro=needs_quad,
            needs_unreal_import=needs_import,
            needs_data_update=needs_data_update,
            import_target_path=import_target,
            data_update_target=f"Content/Data/ArcadeInteractables.json:{row_id}",
            prompt_used=prompt_existing or prompt,
            source_image=_source_target("Interactables", row_id),
            raw_trellis_glb=_target_path("Raw/Trellis/Interactables", row_id, f"{row_id}_Trellis", ".glb"),
            quad_retro_glb=_target_path("Retro/QuadRetro/Interactables", row_id, f"{row_id}_QuadRetro", ".glb"),
            trellis_front_render=_qa_target("TrellisFront", "Interactables", row_id),
            quad_retro_front_render=_qa_target("QuadRetroFront", "Interactables", row_id),
            unreal_asset_path=import_target,
            status=status,
            notes=notes,
        ))
    return entries


def _world_interactable_specs():
    return [
        {
            "row_id": "Crate",
            "display_name": "Loot Item Crate",
            "runtime_class": "AT66CrateInteractable",
            "mesh": "/Game/World/Interactables/Crate.Crate",
            "source_table": "Source/T66/Gameplay/T66CrateInteractable.cpp",
            "data_update_target": "Source/T66/Gameplay/T66CrateInteractable.cpp",
            "import_target": "/Game/World/Interactables/Crate",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "LootCrate.png",
            "note": "Included as the live loot/item crate. Behavior opens the crate HUD and grants a random item; do not convert to random weapon behavior.",
        },
        {
            "row_id": "Chest",
            "display_name": "Treasure Chest",
            "runtime_class": "AT66ChestInteractable",
            "mesh": "/Game/World/Interactables/Chests/ChestModel/Chest.Chest",
            "source_table": "Source/T66/Gameplay/T66ChestInteractable.cpp",
            "data_update_target": "Source/T66/Gameplay/T66ChestInteractable.cpp",
            "import_target": "/Game/World/Interactables/Chests/ChestModel/Chest",
            "previous_key": "Chest",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "TreasureChest.png",
            "note": "Existing Batch01 Trellis/imported mesh is canonical and can be reused for Quad Retro processing.",
        },
        {
            "row_id": "LootBag_Black",
            "display_name": "Black Loot Bag",
            "runtime_class": "AT66LootBagPickup",
            "mesh": "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black",
            "source_table": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "import_target": "/Game/World/LootBags/Black/SM_LootBag_Black",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "LootBag_Black.png",
            "note": "Live rarity loot bag mesh.",
        },
        {
            "row_id": "LootBag_Red",
            "display_name": "Red Loot Bag",
            "runtime_class": "AT66LootBagPickup",
            "mesh": "/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red",
            "source_table": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "import_target": "/Game/World/LootBags/Red/SM_LootBag_Red",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "LootBag_Red.png",
            "note": "Live rarity loot bag mesh.",
        },
        {
            "row_id": "LootBag_Yellow",
            "display_name": "Yellow Loot Bag",
            "runtime_class": "AT66LootBagPickup",
            "mesh": "/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow",
            "source_table": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "import_target": "/Game/World/LootBags/Yellow/SM_LootBag_Yellow",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "LootBag_Yellow.png",
            "note": "Live rarity loot bag mesh.",
        },
        {
            "row_id": "LootBag_White",
            "display_name": "White Loot Bag",
            "runtime_class": "AT66LootBagPickup",
            "mesh": "/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White",
            "source_table": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66LootBagPickup.cpp",
            "import_target": "/Game/World/LootBags/White/SM_LootBag_White",
            "source_hint": "",
            "note": "Live rarity loot bag mesh. No matching source sprite was found in the known mini interactable singles.",
        },
        {
            "row_id": "Fountain",
            "display_name": "Fountain",
            "runtime_class": "AT66FountainInteractable",
            "mesh": "/Game/World/Interactables/Fountain/Fountain.Fountain",
            "source_table": "Source/T66/Gameplay/T66FountainInteractable.cpp",
            "data_update_target": "Source/T66/Gameplay/T66FountainInteractable.cpp",
            "import_target": "/Game/World/Interactables/Fountain/Fountain",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "Fountain.png",
            "note": "Live healing fountain interactable.",
        },
        {
            "row_id": "DifficultyTotem",
            "display_name": "Difficulty Totem",
            "runtime_class": "AT66DifficultyTotem",
            "mesh": "/Game/World/Interactables/Totem.Totem",
            "source_table": "Source/T66/Gameplay/T66DifficultyTotem.cpp",
            "data_update_target": "Source/T66/Gameplay/T66DifficultyTotem.cpp",
            "import_target": "/Game/World/Interactables/Totem",
            "source_hint": "",
            "note": "Live difficulty totem mesh referenced by world and boss-flow code; boss work remains excluded.",
        },
        {
            "row_id": "IdolAltar",
            "display_name": "Idol Altar",
            "runtime_class": "AT66IdolAltar",
            "mesh": "/Game/World/Interactables/SM_IdolAltar.SM_IdolAltar",
            "source_table": "Source/T66/Gameplay/T66IdolAltar.cpp",
            "data_update_target": "Source/T66/Gameplay/T66IdolAltar.cpp",
            "import_target": "/Game/World/Interactables/SM_IdolAltar",
            "source_hint": "",
            "note": "Live idol altar mesh; idol sprite effects are separate and not regenerated here.",
        },
        {
            "row_id": "QuickReviveVending",
            "display_name": "Quick Revive Vending Machine",
            "runtime_class": "AT66QuickReviveVendingMachine",
            "mesh": "/Game/World/Interactables/Vending/Vending.Vending",
            "source_table": "Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp",
            "data_update_target": "Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp",
            "import_target": "/Game/World/Interactables/Vending/Vending",
            "source_hint": PROJECT_ROOT / "SourceAssets" / "Mini" / "Interactables" / "Singles" / "QuickReviveMachine.png",
            "note": "Live quick revive vending interactable.",
        },
        {
            "row_id": "Shroom",
            "display_name": "Bounce Shroom",
            "runtime_class": "AT66Shroom",
            "mesh": "/Game/World/Interactables/Shroom.Shroom",
            "source_table": "Source/T66/Gameplay/T66StageEffects.cpp",
            "data_update_target": "Source/T66/Gameplay/T66StageEffects.cpp",
            "import_target": "/Game/World/Interactables/Shroom",
            "source_hint": "",
            "note": "Referenced stage-effect mesh, but current T66GameMode_WorldInteractables.cpp sets ShroomCount to 0.",
        },
        {
            "row_id": "ArcadeAmplifierPickup",
            "display_name": "Arcade Amplifier Pickup",
            "runtime_class": "AT66ArcadeAmplifierPickup",
            "mesh": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup.ArcadeAmplifierPickup",
            "source_table": "Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp",
            "import_target": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup",
            "previous_key": "ArcadeAmplifierPickup",
            "source_hint": "",
            "note": "Existing Batch01 Trellis/imported reward pickup mesh can be reused for Quad Retro processing.",
        },
        {
            "row_id": "ArcadeAmplifierPickup_Charged",
            "display_name": "Charged Arcade Amplifier Pickup",
            "runtime_class": "AT66ArcadeAmplifierPickup",
            "mesh": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup_Charged.ArcadeAmplifierPickup_Charged",
            "source_table": "Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp",
            "data_update_target": "Source/T66/Gameplay/T66ArcadeAmplifierPickup.cpp",
            "import_target": "/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup_Charged",
            "previous_key": "ArcadeAmplifierPickup_Charged",
            "source_hint": "",
            "note": "Existing Batch01 Trellis/imported charged reward pickup mesh can be reused for Quad Retro processing.",
        },
    ]


def _build_world_entries(previous_entries):
    entries = []
    for spec in _world_interactable_specs():
        previous = previous_entries.get(spec.get("previous_key", spec["row_id"]))
        source_existing, raw_existing, quad_existing, prompt_existing = _previous_interactable_artifacts(previous)
        source_hint = spec.get("source_hint", "")
        if source_hint and Path(source_hint).exists() and not source_existing:
            source_existing = _project_rel(source_hint)

        has_previous_raw = bool(raw_existing)
        needs_source = not source_existing or "SourceAssets/Mini" in source_existing
        needs_trellis = not has_previous_raw
        needs_quad = not quad_existing
        needs_import = needs_quad

        if has_previous_raw:
            quality = "existing_trellis_import_no_quad_retro_glb"
            status = "existing_asset_needs_quad_retro"
            needs_source = False
            needs_trellis = False
        else:
            quality = "existing_unreal_asset_legacy_no_generation_provenance"
            status = "legacy_asset_needs_generation_and_quad_retro"

        prompt = _archetype_prompt(spec["display_name"], "world interactable", "Keep it compact, iconic, and readable from a top-down gameplay camera.")
        entries.append(_entry(
            row_id=spec["row_id"],
            source_table=spec["source_table"],
            category="Interactables",
            display_name=spec["display_name"],
            runtime_class=spec["runtime_class"],
            current_mesh_path=spec["mesh"],
            current_asset_quality=quality,
            existing_generation_run=_project_rel(PREVIOUS_INTERACTABLE_RUN) if previous else "",
            existing_source_image=source_existing,
            existing_raw_trellis_glb=raw_existing,
            existing_quad_retro_glb=quad_existing,
            needs_source_image=needs_source,
            needs_trellis=needs_trellis,
            needs_quad_retro=needs_quad,
            needs_unreal_import=needs_import,
            needs_data_update=False,
            import_target_path=spec["import_target"],
            data_update_target=spec["data_update_target"],
            prompt_used=prompt_existing or prompt,
            source_image=_source_target("Interactables", spec["row_id"]),
            raw_trellis_glb=_target_path("Raw/Trellis/Interactables", spec["row_id"], f"{spec['row_id']}_Trellis", ".glb"),
            quad_retro_glb=_target_path("Retro/QuadRetro/Interactables", spec["row_id"], f"{spec['row_id']}_QuadRetro", ".glb"),
            trellis_front_render=_qa_target("TrellisFront", "Interactables", spec["row_id"]),
            quad_retro_front_render=_qa_target("QuadRetroFront", "Interactables", spec["row_id"]),
            unreal_asset_path=spec["import_target"],
            status=status,
            notes=spec["note"],
        ))
    return entries


def _build_npc_entries(previous_entries):
    house_rows = {row.get("---") or row.get("NPCID"): row for row in _read_csv(HOUSE_NPCS_DATA)}
    visual_rows = {row.get("---"): row for row in _read_csv(CHARACTER_VISUALS_DATA)}
    runtime_by_id = {
        "Gambler": "AT66GamblerNPC",
        "Saint": "AT66SaintNPC",
        "Ouroboros": "AT66OuroborosNPC",
    }
    entries = []
    for row_id in ["Gambler", "Saint", "Ouroboros"]:
        house = house_rows.get(row_id, {})
        visual = visual_rows.get(row_id, {})
        display_name = house.get("DisplayName") or row_id
        current_mesh = visual.get("StaticMesh") or visual.get("SkeletalMesh") or ""
        previous = previous_entries.get("GamblerDemonStand") if row_id == "Gambler" else None
        source_existing, raw_existing, quad_existing, prompt_existing = _previous_interactable_artifacts(previous)
        source_hint = PROJECT_ROOT / "SourceAssets" / "Mini" / "NPCs" / "Singles" / f"{row_id}.png"
        if source_hint.exists() and not source_existing:
            source_existing = _project_rel(source_hint)

        needs_source = not source_existing or "SourceAssets/Mini" in source_existing
        needs_trellis = not raw_existing
        needs_quad = not quad_existing
        needs_import = needs_quad
        import_target = f"/Game/Characters/NPCs/{row_id}/QuadRetro/SM_{row_id}_QuadRetro"
        data_target = f"Content/Data/CharacterVisuals.csv:{row_id}"
        notes = "House NPC row is live. Replace legacy visual data cleanly after a Quad Retro static mesh validates."
        quality = "legacy_skeletal_npc_visual_needs_quad_retro_static_mesh"
        status = "legacy_npc_visual_needs_generation_and_quad_retro"
        if row_id == "Gambler":
            needs_source = False
            needs_trellis = False
            quality = "character_visuals_legacy_skeletal_path_runtime_overrides_to_existing_generated_static_mesh"
            status = "existing_runtime_static_mesh_needs_quad_retro_and_data_cleanup"
            data_target = "Content/Data/CharacterVisuals.csv:Gambler; Source/T66/Gameplay/T66GamblerNPC.cpp"
            notes = "AT66GamblerNPC currently hides the skeletal visual and hard-loads GamblerDemonStand. Reuse that valid Batch01 source/raw model, then remove stale live placeholder paths when the Quad Retro replacement is imported."

        prompt = _archetype_prompt(display_name, "friendly NPC", "Full body, neutral pose, no weapon emphasis, no enemies or bosses.")
        entries.append(_entry(
            row_id=row_id,
            source_table="Content/Data/HouseNPCs.csv; Content/Data/CharacterVisuals.csv",
            category="NPCs",
            display_name=display_name,
            runtime_class=runtime_by_id[row_id],
            current_mesh_path=current_mesh,
            current_asset_quality=quality,
            existing_generation_run=_project_rel(PREVIOUS_INTERACTABLE_RUN) if previous else "",
            existing_source_image=source_existing,
            existing_raw_trellis_glb=raw_existing,
            existing_quad_retro_glb=quad_existing,
            needs_source_image=needs_source,
            needs_trellis=needs_trellis,
            needs_quad_retro=needs_quad,
            needs_unreal_import=needs_import,
            needs_data_update=True,
            import_target_path=import_target,
            data_update_target=data_target,
            prompt_used=prompt_existing or prompt,
            source_image=_source_target("NPCs", row_id),
            raw_trellis_glb=_target_path("Raw/Trellis/NPCs", row_id, f"{row_id}_Trellis", ".glb"),
            quad_retro_glb=_target_path("Retro/QuadRetro/NPCs", row_id, f"{row_id}_QuadRetro", ".glb"),
            trellis_front_render=_qa_target("TrellisFront", "NPCs", row_id),
            quad_retro_front_render=_qa_target("QuadRetroFront", "NPCs", row_id),
            unreal_asset_path=import_target,
            status=status,
            notes=notes,
        ))
    return entries


def _build_floor_wall_entries():
    entries = []
    for module_id in ImportStaticMeshes.COHERENT_THEME_KIT_MODULES:
        asset_name = f"{module_id}_UnrealReady"
        mesh_path = f"{ImportStaticMeshes.COHERENT_THEME_KIT_DEST}/{asset_name}.{asset_name}"
        category = "Walls" if "Wall" in module_id else "Floors"
        parent = _first_static_mesh_material_parent(mesh_path)
        material_ok = parent == EXPECTED_ENV_PARENT
        quality = "coherent_theme_kit_imported"
        status = "already_valid_world_kit_retro_material_path" if material_ok else "questionable_world_kit_material_parent"
        notes = "Existing CoherentThemeKit01 module. Do not regenerate; floor/wall retro treatment should stay in existing Retro FX/world geometry and M_Environment_Unlit material flow."
        entries.append(_entry(
            row_id=module_id,
            source_table="Scripts/ImportStaticMeshes.py:COHERENT_THEME_KIT_MODULES; Source/T66/Gameplay/T66TowerThemeVisuals.cpp",
            category=category,
            display_name=module_id,
            runtime_class="FT66TowerThemeVisuals/T66TowerMapTerrain/T66PreviewStageEnvironment",
            current_mesh_path=mesh_path,
            current_asset_quality=f"{quality}; expected_material_parent={EXPECTED_ENV_PARENT}; observed_material_parent={parent or 'none'}",
            existing_generation_run=_project_rel(PREVIOUS_ENV_RUN),
            existing_source_image=_find_env_artifact(module_id, "source"),
            existing_raw_trellis_glb=_find_env_artifact(module_id, "raw"),
            existing_quad_retro_glb="",
            needs_source_image=False,
            needs_trellis=False,
            needs_quad_retro=False,
            needs_unreal_import=False,
            needs_data_update=False,
            import_target_path=_package_path(mesh_path),
            data_update_target="Source/T66/Gameplay/T66TowerThemeVisuals.cpp",
            prompt_used="Existing world kit module; no new Trellis prompt needed for this batch.",
            source_image="",
            raw_trellis_glb="",
            quad_retro_glb="",
            trellis_front_render="",
            quad_retro_front_render="",
            unreal_asset_path=_package_path(mesh_path),
            status=status,
            notes=notes,
        ))
    return entries


def _props_summary():
    rows = _read_csv(PROPS_DATA)
    live_rows = [row for row in rows if row.get("---")]
    return {
        "source_table": "Content/Data/Props.csv",
        "live_rows": len(live_rows),
        "status": "retired_no_live_rows",
        "notes": "Props were explicitly retired before this continuation. No prop models are generated by this batch.",
    }


def _summary(entries):
    def count_if(key):
        return sum(1 for entry in entries if entry.get(key))

    already_valid = sum(
        1
        for entry in entries
        if not entry["needs_source_image"]
        and not entry["needs_trellis"]
        and not entry["needs_quad_retro"]
        and not entry["needs_unreal_import"]
        and entry["current_asset_exists"]
    )
    questionable = sum(1 for entry in entries if "questionable" in entry["status"] or not entry["current_asset_exists"])
    by_category = {}
    by_status = {}
    for entry in entries:
        by_category[entry["category"]] = by_category.get(entry["category"], 0) + 1
        by_status[entry["status"]] = by_status.get(entry["status"], 0) + 1
    return {
        "total_audited_rows": len(entries),
        "already_valid": already_valid,
        "needs_source_image": count_if("needs_source_image"),
        "needs_trellis": count_if("needs_trellis"),
        "needs_quad_retro": count_if("needs_quad_retro"),
        "needs_unreal_import": count_if("needs_unreal_import"),
        "needs_data_update": count_if("needs_data_update"),
        "failed_or_questionable": questionable,
        "by_category": by_category,
        "by_status": by_status,
    }


def _write_status(report):
    status_path = NOTES_DIR / "STATUS.md"
    lines = [
        "# WorldNpcInteractablesRetroBatch01 Status",
        "",
        f"- Manifest generated: {datetime.now().isoformat(timespec='seconds')}",
        f"- Output root: {OUTPUT_ROOT}",
        f"- Total audited rows: {report['summary']['total_audited_rows']}",
        f"- Props live rows: {report['props']['live_rows']} ({report['props']['status']})",
        f"- Needs source images: {report['summary']['needs_source_image']}",
        f"- Needs Trellis: {report['summary']['needs_trellis']}",
        f"- Needs Quad Retro: {report['summary']['needs_quad_retro']}",
        f"- Needs Unreal import: {report['summary']['needs_unreal_import']}",
        "",
        "Scope guard:",
        "- Regular enemies excluded.",
        "- Bosses excluded.",
        "- Weapon projectile meshes excluded unless future validation finds a broken reference.",
        "- Loot crate included as AT66CrateInteractable item/loot crate visual, not a random-weapon behavior change.",
        "- Floors/walls audited from CoherentThemeKit01 without Trellis regeneration.",
    ]
    with open(status_path, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")


def main():
    REPORTS_DIR.mkdir(parents=True, exist_ok=True)
    NOTES_DIR.mkdir(parents=True, exist_ok=True)

    previous_entries = _read_previous_interactable_entries()
    entries = []
    entries.extend(_build_arcade_entries(previous_entries))
    entries.extend(_build_world_entries(previous_entries))
    entries.extend(_build_npc_entries(previous_entries))
    entries.extend(_build_floor_wall_entries())

    report = {
        "manifest_name": "MissingInteractablesManifest",
        "generated_at": datetime.now().isoformat(timespec="seconds"),
        "output_root": str(OUTPUT_ROOT),
        "scope": {
            "included": [
                "ArcadeInteractables.json rows",
                "world interactable runtime mesh references",
                "HouseNPCs/CharacterVisuals NPC rows",
                "CoherentThemeKit01 floor/wall modules",
            ],
            "excluded": [
                "regular enemies",
                "bosses",
                "weapon projectile meshes unless validation later finds a broken reference",
                "props, because Props.csv has no live rows after requested props retirement",
            ],
        },
        "props": _props_summary(),
        "summary": _summary(entries),
        "entries": entries,
    }

    manifest_path = REPORTS_DIR / "MissingInteractablesManifest.json"
    with open(manifest_path, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(report, handle, indent=2)
        handle.write("\n")

    _write_status(report)

    unreal.log(f"[BuildWorldNpcInteractablesRetroBatch01Manifest] wrote {manifest_path}")
    unreal.log(f"[BuildWorldNpcInteractablesRetroBatch01Manifest] summary={report['summary']}")

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"[BuildWorldNpcInteractablesRetroBatch01Manifest] Failed to request QUIT_EDITOR: {exc}")


if __name__ == "__main__":
    main()
