"""
Read-only audit for HillTile / CliffSideMaterials cleanup.

This verifies whether Blueprint/content data appears to override or consume the
AT66GameMode CliffSideMaterials editable property before HillTile defaults are
removed from native code.
"""

import json
from datetime import datetime, timezone
from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUTPUT_PATH = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-05-27" / "cliff_side_materials_audit.json"
SCAN_ROOTS = ["/Game", "/Game/Maps"]
TARGET_NATIVE_CLASS = "T66GameMode"
PROPERTY_NAME = "CliffSideMaterials"
TOKENS = [PROPERTY_NAME, "MI_HillTile", "T_HillTile", "/Game/World/Cliffs"]
TARGET_CONTENT_FILES = {
    "Content/World/Cliffs/MI_HillTile1.uasset",
    "Content/World/Cliffs/MI_HillTile2.uasset",
    "Content/World/Cliffs/MI_HillTile3.uasset",
    "Content/World/Cliffs/MI_HillTile4.uasset",
    "Content/World/Cliffs/T_HillTile1.uasset",
    "Content/World/Cliffs/T_HillTile2.uasset",
    "Content/World/Cliffs/T_HillTile3.uasset",
    "Content/World/Cliffs/T_HillTile4.uasset",
}


def log(message):
    unreal.log(f"[AuditCliffSideMaterials] {message}")


def force_scan():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        registry.scan_paths_synchronous(SCAN_ROOTS, True)
    except TypeError:
        registry.scan_paths_synchronous(SCAN_ROOTS)
    try:
        registry.wait_for_completion()
    except Exception:
        pass
    return registry


def asset_data_path(asset_data):
    try:
        return str(asset_data.object_path)
    except Exception:
        package = str(asset_data.package_name)
        name = str(asset_data.asset_name)
        return f"{package}.{name}"


def asset_data_package(asset_data):
    try:
        return str(asset_data.package_name)
    except Exception:
        return asset_data_path(asset_data).split(".", 1)[0]


def asset_data_class(asset_data):
    for prop in ("asset_class_path", "asset_class"):
        try:
            value = getattr(asset_data, prop)
            if value:
                return str(value)
        except Exception:
            pass
    return ""


def asset_tags_text(asset_data):
    chunks = []
    try:
        chunks.append(str(asset_data.tags_and_values))
    except Exception:
        pass
    for tag_name in ("GeneratedClass", "ParentClass", "NativeParentClass", "BlueprintParentClass"):
        try:
            value = asset_data.get_tag_value(tag_name)
            if value:
                chunks.append(str(value))
        except Exception:
            pass
    return "\n".join(chunks)


def class_matches(loaded_class, target_class):
    if not loaded_class or not target_class:
        return False
    try:
        current = loaded_class
        while current:
            if current == target_class or current.get_name() == target_class.get_name():
                return True
            current = current.get_super_class()
    except Exception:
        return False
    return False


def get_blueprint_classes(asset):
    classes = []
    for prop in ("generated_class", "parent_class", "skeleton_generated_class"):
        try:
            value = asset.get_editor_property(prop)
            if value:
                classes.append(value)
        except Exception:
            pass
    return classes


def soft_object_paths(values):
    rows = []
    try:
        iterable = list(values)
    except Exception:
        iterable = []
    for value in iterable:
        try:
            path = value.to_soft_object_path()
            rows.append(str(path))
        except Exception:
            rows.append(str(value))
    return rows


def audit_game_mode_blueprints(registry):
    target_class = unreal.load_class(None, f"/Script/T66.{TARGET_NATIVE_CLASS}")
    if not target_class:
        raise RuntimeError(f"Failed to resolve /Script/T66.{TARGET_NATIVE_CLASS}")

    matches = []
    load_failures = []
    all_assets = registry.get_all_assets()
    game_assets = [asset for asset in all_assets if asset_data_package(asset).startswith("/Game")]
    for asset_data in game_assets:
        class_text = asset_data_class(asset_data)
        tags_text = asset_tags_text(asset_data)
        if "Blueprint" not in class_text and "Blueprint" not in tags_text:
            continue
        if TARGET_NATIVE_CLASS not in tags_text and TARGET_NATIVE_CLASS not in asset_data_path(asset_data):
            continue

        asset_path = asset_data_path(asset_data)
        try:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        except Exception as exc:
            load_failures.append({"asset": asset_path, "error": str(exc)})
            continue
        if not asset:
            load_failures.append({"asset": asset_path, "error": "load_asset returned null"})
            continue

        matched_classes = []
        for bp_class in get_blueprint_classes(asset):
            if class_matches(bp_class, target_class):
                matched_classes.append(bp_class)

        for bp_class in matched_classes:
            cdo_paths = []
            cdo_error = None
            try:
                cdo = unreal.get_default_object(bp_class)
                values = cdo.get_editor_property(PROPERTY_NAME)
                cdo_paths = soft_object_paths(values)
            except Exception as exc:
                cdo_error = str(exc)
            matches.append(
                {
                    "asset": asset_path,
                    "class": bp_class.get_name(),
                    "cliff_side_materials": cdo_paths,
                    "contains_hilltile_default": any("HillTile" in path for path in cdo_paths),
                    "cdo_error": cdo_error,
                }
            )

    return {
        "target_class": f"/Script/T66.{TARGET_NATIVE_CLASS}",
        "matches": matches,
        "load_failures": load_failures,
    }


def scan_content_tokens():
    content_root = PROJECT_ROOT / "Content"
    matches = []
    failures = []
    encoded = {token: [token.encode("utf-8", errors="ignore"), token.encode("utf-16-le", errors="ignore")] for token in TOKENS}
    for path in content_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".uasset", ".umap"}:
            continue
        try:
            data = path.read_bytes()
        except Exception as exc:
            failures.append({"path": str(path.relative_to(PROJECT_ROOT)), "error": str(exc)})
            continue
        hits = [token for token, patterns in encoded.items() if any(pattern in data for pattern in patterns)]
        if hits:
            rel_path = str(path.relative_to(PROJECT_ROOT)).replace("\\", "/")
            matches.append(
                {
                    "path": rel_path,
                    "hits": sorted(set(hits)),
                    "is_target_hilltile_file": rel_path in TARGET_CONTENT_FILES,
                }
            )
    return {"matches": matches, "read_failures": failures}


def main():
    registry = force_scan()
    report = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "scan_roots": SCAN_ROOTS,
        "game_mode_blueprints": audit_game_mode_blueprints(registry),
        "content_token_scan": scan_content_tokens(),
    }
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    non_target_hits = [row for row in report["content_token_scan"]["matches"] if not row["is_target_hilltile_file"]]
    log(
        f"bp_matches={len(report['game_mode_blueprints']['matches'])} "
        f"content_hits={len(report['content_token_scan']['matches'])} "
        f"non_target_content_hits={len(non_target_hits)} output={OUTPUT_PATH}"
    )
    if report["game_mode_blueprints"]["load_failures"]:
        raise RuntimeError("Blueprint load failures occurred during CliffSideMaterials audit")
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        unreal.log_warning(f"[AuditCliffSideMaterials] Failed to request QUIT_EDITOR: {exc}")


main()
