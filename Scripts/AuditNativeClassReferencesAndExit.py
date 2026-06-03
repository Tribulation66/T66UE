"""
Read-only audit for native C++ class/member references in content and text files.

This script is intentionally conservative. It combines:
- native class resolution under /Script/T66
- AssetRegistry tag scans
- Blueprint parent/generated-class tag inspection, with optional load confirmation
- best-effort live game map actor enumeration under configured map roots
- raw Content package string scans for class/member tokens
- text scans across Source, Config, Content/Data, Scripts, and Reports

Run with UnrealEditor-Cmd:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=pythonscript ^
    -script="C:/UE/T66/Scripts/AuditNativeClassReferencesAndExit.py" ^
    -T66AuditClasses=AT66StageGate,AT66IdolAltar ^
    -T66AuditMembers=ExampleMemberName ^
    -T66AuditMapRoots=/Game/Maps ^
    -T66AuditOutput="C:/UE/T66/Reports/Hygiene/2026-05-27/native_reference_audit.json" ^
    -unattended -nop4 -nosplash -log
"""

import json
import re
from pathlib import Path

import unreal


LOG = "[AuditNativeRefs]"
PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
DEFAULT_OUTPUT = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-05-27" / "native_reference_audit.json"

DEFAULT_CLASSES = []

DEFAULT_MEMBERS = []

TEXT_SCAN_DIRS = ["Source", "Config", "Content/Data", "Scripts", "Reports"]
TEXT_EXTENSIONS = {".cpp", ".h", ".hpp", ".inl", ".py", ".ps1", ".ini", ".csv", ".json", ".txt", ".md"}
CONTENT_BINARY_EXTENSIONS = {".uasset", ".umap"}
DEFAULT_MAP_ROOTS = ["/Game/Maps"]


def log(message):
    unreal.log(f"{LOG} {message}")


def warn(message):
    unreal.log_warning(f"{LOG} {message}")


def get_command_line():
    try:
        return unreal.SystemLibrary.get_command_line()
    except Exception:
        return ""


def get_param(name, default=None):
    command_line = get_command_line()
    match = re.search(rf"-{re.escape(name)}=(\"[^\"]*\"|\S+)", command_line)
    if not match:
        return default
    value = match.group(1)
    if value.startswith('"') and value.endswith('"'):
        return value[1:-1]
    return value


def split_csv(value, default):
    if not value:
        return list(default)
    return [part.strip() for part in value.split(",") if part.strip()]


def strip_unreal_prefix(name):
    if len(name) > 1 and name[0] in ("A", "U", "F") and name[1].isupper():
        return name[1:]
    return name


def class_script_names(class_name):
    stripped = strip_unreal_prefix(class_name)
    names = {class_name, stripped}
    return sorted(names)


def load_native_class(class_name):
    candidates = []
    for script_name in class_script_names(class_name):
        candidates.append(f"/Script/T66.{script_name}")
        candidates.append(f"/Script/T66.{class_name}")
    for path in dict.fromkeys(candidates):
        try:
            loaded = unreal.load_class(None, path)
            if loaded:
                return loaded, path
        except Exception:
            pass
        try:
            loaded_obj = unreal.load_object(None, path)
            if loaded_obj:
                return loaded_obj, path
        except Exception:
            pass
    return None, None


def asset_data_path(asset_data):
    try:
        return str(asset_data.object_path)
    except Exception:
        pass
    try:
        package = str(asset_data.package_name)
        name = str(asset_data.asset_name)
        return f"{package}.{name}"
    except Exception:
        return str(asset_data)


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
    try:
        return str(asset_data.get_editor_property("asset_class"))
    except Exception:
        return ""


def asset_tags_text(asset_data):
    chunks = []
    try:
        tags = asset_data.tags_and_values
        chunks.append(str(tags))
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
            if current == target_class:
                return True
            current_name = current.get_name()
            target_name = target_class.get_name()
            if current_name == target_name:
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


def scan_asset_registry(all_assets, class_targets, member_targets):
    results = []
    tokens = []
    for name, meta in class_targets.items():
        tokens.extend([name, meta.get("script_path") or ""])
        tokens.extend(class_script_names(name))
    tokens.extend(member_targets)
    tokens = [token for token in dict.fromkeys(tokens) if token]

    for asset_data in all_assets:
        text = "\n".join([asset_data_path(asset_data), asset_data_package(asset_data), asset_data_class(asset_data), asset_tags_text(asset_data)])
        hits = [token for token in tokens if token and token in text]
        if hits:
            results.append(
                {
                    "asset": asset_data_path(asset_data),
                    "package": asset_data_package(asset_data),
                    "asset_class": asset_data_class(asset_data),
                    "hits": sorted(set(hits)),
                }
            )
    return results


def is_generated_class_asset(asset_path, class_text):
    return (
        asset_path.endswith("_C")
        or "_C." in asset_path
        or "BlueprintGeneratedClass" in class_text
        or "SkeletonGeneratedClass" in class_text
    )


def blueprint_tag_hits(asset_data, class_targets):
    text = "\n".join([asset_data_path(asset_data), asset_data_class(asset_data), asset_tags_text(asset_data)])
    hits = []
    for target_name, meta in class_targets.items():
        tokens = [target_name, meta.get("script_name") or "", meta.get("script_path") or ""]
        tokens.extend(class_script_names(target_name))
        if any(token and token in text for token in tokens):
            hits.append(target_name)
    return sorted(set(hits))


def scan_blueprints(all_assets, class_targets):
    results = []
    load_failures = []
    scanned = 0
    candidates = 0
    for asset_data in all_assets:
        class_text = asset_data_class(asset_data)
        tags_text = asset_tags_text(asset_data)
        if "Blueprint" not in class_text and "Blueprint" not in tags_text:
            continue
        asset_path = asset_data_path(asset_data)
        tag_hits = blueprint_tag_hits(asset_data, class_targets)
        if not tag_hits:
            continue
        candidates += 1
        if is_generated_class_asset(asset_path, class_text):
            results.append({"asset": asset_path, "class_hits": tag_hits, "evidence": "asset_registry_tags"})
            continue

        scanned += 1
        try:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        except Exception as exc:
            load_failures.append({"asset": asset_path, "error": str(exc)})
            results.append({"asset": asset_path, "class_hits": tag_hits, "evidence": "asset_registry_tags_load_failed"})
            continue
        if not asset:
            load_failures.append({"asset": asset_path, "error": "load_asset returned null"})
            results.append({"asset": asset_path, "class_hits": tag_hits, "evidence": "asset_registry_tags_load_null"})
            continue

        class_hits = []
        for bp_class in get_blueprint_classes(asset):
            for target_name, meta in class_targets.items():
                target_class = meta.get("class_object")
                if class_matches(bp_class, target_class):
                    class_hits.append(target_name)

        if class_hits:
            results.append({"asset": asset_path, "class_hits": sorted(set(class_hits)), "evidence": "loaded_blueprint_parent"})
        else:
            results.append({"asset": asset_path, "class_hits": tag_hits, "evidence": "asset_registry_tags_unconfirmed"})
    return {"candidates": candidates, "loaded": scanned, "matches": results, "load_failures": load_failures}


def scan_maps(all_assets, class_targets, map_roots):
    maps = []
    roots = [root.rstrip("/") for root in map_roots if root]
    for asset_data in all_assets:
        class_text = asset_data_class(asset_data)
        path = asset_data_path(asset_data)
        if "World" in class_text or path.endswith(".World") or asset_data_package(asset_data).lower().endswith("_map"):
            package = asset_data_package(asset_data)
            if package.startswith("/Game") and any(package == root or package.startswith(f"{root}/") for root in roots):
                maps.append(package)

    matches = []
    failures = []
    scanned = 0
    actor_subsystem = None
    try:
        actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    except Exception:
        actor_subsystem = None

    for map_package in sorted(set(maps)):
        try:
            unreal.EditorLoadingAndSavingUtils.load_map(map_package)
            scanned += 1
            if actor_subsystem:
                actors = actor_subsystem.get_all_level_actors()
            else:
                actors = unreal.EditorLevelLibrary.get_all_level_actors()
            for actor in actors:
                try:
                    actor_class = actor.get_class()
                except Exception:
                    continue
                class_hits = []
                for target_name, meta in class_targets.items():
                    if class_matches(actor_class, meta.get("class_object")):
                        class_hits.append(target_name)
                if class_hits:
                    matches.append(
                        {
                            "map": map_package,
                            "actor": actor.get_name(),
                            "actor_class": actor_class.get_name(),
                            "class_hits": sorted(set(class_hits)),
                        }
                    )
        except Exception as exc:
            failures.append({"map": map_package, "error": str(exc)})
    return {"candidate_maps": len(set(maps)), "scanned": scanned, "matches": matches, "load_failures": failures}


def file_contains_any(path, patterns):
    try:
        data = path.read_bytes()
    except Exception as exc:
        return None, str(exc)
    hits = []
    for token, encoded_patterns in patterns.items():
        if any(pattern in data for pattern in encoded_patterns):
            hits.append(token)
    return sorted(hits), None


def scan_binary_content(class_targets, member_targets):
    tokens = []
    for class_name, meta in class_targets.items():
        tokens.extend([class_name, meta.get("script_name") or "", meta.get("script_path") or ""])
        tokens.extend(class_script_names(class_name))
    tokens.extend(member_targets)
    tokens = [token for token in dict.fromkeys(tokens) if token]
    patterns = {}
    for token in tokens:
        encoded = [token.encode("utf-8", errors="ignore")]
        try:
            encoded.append(token.encode("utf-16-le", errors="ignore"))
        except Exception:
            pass
        patterns[token] = encoded

    scanned = 0
    matches = []
    failures = []
    content_root = PROJECT_ROOT / "Content"
    if not content_root.exists():
        return {"scanned": 0, "matches": [], "read_failures": [{"path": str(content_root), "error": "missing"}]}

    for path in content_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in CONTENT_BINARY_EXTENSIONS:
            continue
        scanned += 1
        hits, error = file_contains_any(path, patterns)
        if error:
            failures.append({"path": str(path.relative_to(PROJECT_ROOT)), "error": error})
        elif hits:
            matches.append({"path": str(path.relative_to(PROJECT_ROOT)), "hits": hits})
    return {"scanned": scanned, "matches": matches, "read_failures": failures}


def scan_text_files(class_targets, member_targets):
    tokens = []
    for class_name, meta in class_targets.items():
        tokens.extend([class_name, meta.get("script_name") or "", meta.get("script_path") or ""])
        tokens.extend(class_script_names(class_name))
    tokens.extend(member_targets)
    tokens = [token for token in dict.fromkeys(tokens) if token]

    scanned = 0
    matches = []
    failures = []
    for rel_dir in TEXT_SCAN_DIRS:
        root = PROJECT_ROOT / rel_dir
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in TEXT_EXTENSIONS:
                continue
            scanned += 1
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except Exception as exc:
                failures.append({"path": str(path.relative_to(PROJECT_ROOT)), "error": str(exc)})
                continue
            hits = [token for token in tokens if token and token in text]
            if hits:
                matches.append({"path": str(path.relative_to(PROJECT_ROOT)), "hits": sorted(set(hits))})
    return {"scanned": scanned, "matches": matches, "read_failures": failures}


def build_class_targets(class_names):
    targets = {}
    for class_name in class_names:
        class_obj, script_path = load_native_class(class_name)
        targets[class_name] = {
            "resolved": bool(class_obj),
            "script_path": script_path,
            "script_name": script_path.rsplit(".", 1)[-1] if script_path else None,
            "class_object": class_obj,
            "class_object_name": class_obj.get_name() if class_obj else None,
        }
    return targets


def serializable_class_targets(class_targets):
    rows = {}
    for key, meta in class_targets.items():
        rows[key] = {k: v for k, v in meta.items() if k != "class_object"}
    return rows


def main():
    class_names = split_csv(get_param("T66AuditClasses"), DEFAULT_CLASSES)
    member_names = split_csv(get_param("T66AuditMembers"), DEFAULT_MEMBERS)
    map_roots = split_csv(get_param("T66AuditMapRoots"), DEFAULT_MAP_ROOTS)
    output_path = Path(get_param("T66AuditOutput", str(DEFAULT_OUTPUT)))
    if not output_path.is_absolute():
        output_path = PROJECT_ROOT / output_path

    log(f"classes={class_names}")
    log(f"members={member_names}")
    log(f"map_roots={map_roots}")

    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    all_assets = registry.get_all_assets()
    game_assets = [asset for asset in all_assets if asset_data_package(asset).startswith("/Game")]
    class_targets = build_class_targets(class_names)

    report = {
        "project_root": str(PROJECT_ROOT),
        "map_roots": map_roots,
        "classes": serializable_class_targets(class_targets),
        "members": member_names,
        "asset_count_all": len(all_assets),
        "asset_count_game": len(game_assets),
        "asset_registry_hits": scan_asset_registry(game_assets, class_targets, member_names),
        "blueprints": scan_blueprints(game_assets, class_targets),
        "maps": scan_maps(game_assets, class_targets, map_roots),
        "binary_content": scan_binary_content(class_targets, member_names),
        "text": scan_text_files(class_targets, member_names),
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    log(
        "complete "
        f"assets={report['asset_count_game']} "
        f"bp_candidates={report['blueprints']['candidates']} "
        f"bp_loaded={report['blueprints']['loaded']} "
        f"maps_scanned={report['maps']['scanned']}/{report['maps']['candidate_maps']} "
        f"binary_scanned={report['binary_content']['scanned']} "
        f"text_scanned={report['text']['scanned']} "
        f"output={output_path}"
    )

    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        warn(f"Failed to request QUIT_EDITOR: {exc}")


main()
