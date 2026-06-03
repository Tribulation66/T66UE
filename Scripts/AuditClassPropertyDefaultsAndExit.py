"""
Audit serialized/default property references before removing UPROPERTY surfaces.

The report combines text/binary token scans with best-effort native and
Blueprint CDO inspection. It is intentionally read-only.
"""

import json
import re
from datetime import datetime, timezone
from pathlib import Path

import unreal


LOG = "[AuditClassProps]"
PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
DEFAULT_OUTPUT = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-05-27" / "class_property_audit.json"
TEXT_SCAN_DIRS = ["Source", "Config", "Content/Data", "Scripts", "Reports", "Gameplay", "UI"]
TEXT_EXTENSIONS = {".cpp", ".h", ".hpp", ".inl", ".py", ".ps1", ".ini", ".csv", ".json", ".txt", ".md", ".uproject", ".uplugin"}
CONTENT_BINARY_EXTENSIONS = {".uasset", ".umap"}


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


def split_csv(value):
    if not value:
        return []
    return [part.strip() for part in value.split(",") if part.strip()]


def strip_unreal_prefix(name):
    if len(name) > 1 and name[0] in ("A", "U", "F") and name[1].isupper():
        return name[1:]
    return name


def class_script_names(class_name):
    stripped = strip_unreal_prefix(class_name)
    return sorted({class_name, stripped})


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


def force_asset_registry_scan():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        registry.scan_paths_synchronous(["/Game"], True)
    except TypeError:
        registry.scan_paths_synchronous(["/Game"])
    try:
        registry.wait_for_completion()
    except Exception:
        pass
    return registry, {
        "forced_asset_registry_scan": True,
        "scan_roots": ["/Game"],
        "generated_utc": datetime.now(timezone.utc).isoformat(),
    }


def asset_data_path(asset_data):
    try:
        return str(asset_data.object_path)
    except Exception:
        try:
            package = str(asset_data.package_name)
            name = str(asset_data.asset_name)
            return f"{package}.{name}"
        except Exception:
            return str(asset_data)


def asset_data_class(asset_data):
    try:
        return str(asset_data.asset_class_path)
    except Exception:
        try:
            return str(asset_data.asset_class)
        except Exception:
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


def get_cdo(class_obj):
    if not class_obj:
        return None
    for expr in (
        lambda: unreal.get_default_object(class_obj),
        lambda: class_obj.get_default_object(),
    ):
        try:
            value = expr()
            if value:
                return value
        except Exception:
            pass
    return None


def serialize_value(value):
    try:
        if isinstance(value, (str, int, float, bool)) or value is None:
            return value
    except Exception:
        pass
    try:
        return str(value)
    except Exception:
        return "<unserializable>"


def inspect_object_properties(obj, properties):
    rows = []
    for prop in properties:
        row = {"property": prop, "readable": False, "value": None, "error": None}
        try:
            value = obj.get_editor_property(prop)
            row["readable"] = True
            row["value"] = serialize_value(value)
        except Exception as exc:
            row["error"] = str(exc)
        rows.append(row)
    return rows


def scan_blueprint_cdos(all_assets, class_targets, properties):
    matches = []
    failures = []
    candidates = 0
    for asset_data in all_assets:
        class_text = asset_data_class(asset_data)
        tags_text = asset_tags_text(asset_data)
        if "Blueprint" not in class_text and "Blueprint" not in tags_text:
            continue
        asset_path = asset_data_path(asset_data)
        combined_text = "\n".join([asset_path, class_text, tags_text])
        if not any(token and token in combined_text for meta in class_targets.values() for token in meta["tokens"]):
            continue
        candidates += 1
        try:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        except Exception as exc:
            failures.append({"asset": asset_path, "error": str(exc)})
            continue
        if not asset:
            failures.append({"asset": asset_path, "error": "load_asset returned null"})
            continue
        for bp_class in get_blueprint_classes(asset):
            for target_name, meta in class_targets.items():
                if not class_matches(bp_class, meta["class_object"]):
                    continue
                cdo = get_cdo(bp_class)
                matches.append(
                    {
                        "asset": asset_path,
                        "target_class": target_name,
                        "blueprint_class": bp_class.get_name() if bp_class else None,
                        "cdo_properties": inspect_object_properties(cdo, properties) if cdo else [],
                        "cdo_loaded": bool(cdo),
                    }
                )
    return {"candidates": candidates, "matches": matches, "load_failures": failures}


def build_patterns(tokens):
    patterns = {}
    for token in tokens:
        encoded = [token.encode("utf-8", errors="ignore")]
        try:
            encoded.append(token.encode("utf-16-le", errors="ignore"))
        except Exception:
            pass
        patterns[token] = encoded
    return patterns


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


def scan_binary(tokens):
    patterns = build_patterns(tokens)
    scanned = 0
    matches = []
    failures = []
    content_root = PROJECT_ROOT / "Content"
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


def scan_text(tokens):
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


def main():
    class_names = split_csv(get_param("T66AuditClasses"))
    properties = split_csv(get_param("T66AuditProperties"))
    output_path = Path(get_param("T66AuditOutput", str(DEFAULT_OUTPUT)))
    if not output_path.is_absolute():
        output_path = PROJECT_ROOT / output_path
    if not properties:
        raise RuntimeError("No properties supplied through -T66AuditProperties")

    registry, metadata = force_asset_registry_scan()
    all_assets = [asset for asset in registry.get_all_assets() if str(asset.package_name).startswith("/Game")]

    class_targets = {}
    native_cdos = []
    for class_name in class_names:
        class_obj, script_path = load_native_class(class_name)
        tokens = class_script_names(class_name)
        if script_path:
            tokens.append(script_path)
        class_targets[class_name] = {"class_object": class_obj, "script_path": script_path, "tokens": tokens}
        cdo = get_cdo(class_obj)
        native_cdos.append(
            {
                "class": class_name,
                "resolved": bool(class_obj),
                "script_path": script_path,
                "cdo_loaded": bool(cdo),
                "cdo_properties": inspect_object_properties(cdo, properties) if cdo else [],
            }
        )

    tokens = [token for token in dict.fromkeys(properties + [t for meta in class_targets.values() for t in meta["tokens"]]) if token]
    report = {
        "project_root": str(PROJECT_ROOT),
        "metadata": metadata,
        "classes": [
            {
                "class": name,
                "resolved": bool(meta["class_object"]),
                "script_path": meta["script_path"],
            }
            for name, meta in class_targets.items()
        ],
        "properties": properties,
        "tokens": tokens,
        "native_cdos": native_cdos,
        "blueprint_cdos": scan_blueprint_cdos(all_assets, class_targets, properties) if class_targets else {"candidates": 0, "matches": [], "load_failures": []},
        "binary_content": scan_binary(tokens),
        "text": scan_text(tokens),
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    log(
        f"complete classes={len(class_names)} properties={len(properties)} "
        f"bp_matches={len(report['blueprint_cdos']['matches'])} "
        f"binary_matches={len(report['binary_content']['matches'])} "
        f"text_matches={len(report['text']['matches'])} output={output_path}"
    )
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        warn(f"Failed to request QUIT_EDITOR: {exc}")


main()
