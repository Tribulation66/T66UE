"""
Audit exact Unreal asset package references.

This read-only helper is used for cleanup gates where filename matching is not
enough. It checks AssetRegistry/package referencers plus text and raw binary
token matches for an explicit package list.
"""

import json
import re
from datetime import datetime, timezone
from pathlib import Path

import unreal


LOG = "[AuditAssetRefs]"
PROJECT_ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
DEFAULT_OUTPUT = PROJECT_ROOT / "Reports" / "Hygiene" / "2026-05-27" / "asset_reference_audit.json"
TEXT_SCAN_DIRS = ["Source", "Config", "Content/Data", "Scripts", "Reports", "Gameplay", "UI", "ToonStyle", "Model Generation"]
TEXT_EXTENSIONS = {".cpp", ".h", ".hpp", ".inl", ".py", ".ps1", ".ini", ".csv", ".json", ".txt", ".md", ".uplugin", ".uproject"}
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


def normalize_package(value):
    value = value.strip().replace("\\", "/")
    if "." in value:
        value = value.split(".", 1)[0]
    return value.rstrip("/")


def package_to_asset_path(package):
    name = package.rsplit("/", 1)[-1]
    return f"{package}.{name}"


def package_scan_roots(packages):
    roots = set()
    for package in packages:
        parts = package.split("/")
        if len(parts) >= 3 and parts[1] == "Game":
            roots.add("/".join(parts[:3]))
        elif package.startswith("/Game"):
            roots.add("/Game")
    return sorted(roots) or ["/Game"]


def force_asset_registry_scan(packages):
    roots = package_scan_roots(packages)
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        registry.scan_paths_synchronous(roots, True)
    except TypeError:
        registry.scan_paths_synchronous(roots)
    try:
        registry.wait_for_completion()
    except Exception:
        pass
    return registry, {
        "forced_asset_registry_scan": True,
        "scan_roots": roots,
        "generated_utc": datetime.now(timezone.utc).isoformat(),
    }


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


def scan_binary(tokens):
    patterns = build_patterns(tokens)
    scanned = 0
    matches = []
    failures = []
    content_root = PROJECT_ROOT / "Content"
    if not content_root.exists():
        return {"scanned": 0, "matches": [], "read_failures": [{"path": "Content", "error": "missing"}]}
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


def get_referencers(asset_path):
    try:
        refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(asset_path, True)
        return sorted(str(ref) for ref in refs)
    except Exception as exc:
        return {"error": str(exc)}


def asset_data_for_package(registry_assets, package):
    rows = []
    for asset_data in registry_assets:
        try:
            package_name = str(asset_data.package_name)
        except Exception:
            continue
        if package_name != package:
            continue
        try:
            asset_path = str(asset_data.object_path)
        except Exception:
            name = str(getattr(asset_data, "asset_name", package.rsplit("/", 1)[-1]))
            asset_path = f"{package}.{name}"
        class_name = ""
        try:
            class_name = str(asset_data.asset_class_path)
        except Exception:
            try:
                class_name = str(asset_data.asset_class)
            except Exception:
                class_name = ""
        rows.append({"asset": asset_path, "asset_class": class_name})
    return rows


def main():
    packages = [normalize_package(value) for value in split_csv(get_param("T66AuditPackages"))]
    packages = [package for package in dict.fromkeys(packages) if package.startswith("/Game/")]
    extra_tokens = split_csv(get_param("T66AuditTokens"))
    output_path = Path(get_param("T66AuditOutput", str(DEFAULT_OUTPUT)))
    if not output_path.is_absolute():
        output_path = PROJECT_ROOT / output_path

    if not packages:
        raise RuntimeError("No /Game package paths supplied through -T66AuditPackages")

    registry, metadata = force_asset_registry_scan(packages)
    all_assets = registry.get_all_assets()
    tokens = []
    for package in packages:
        asset_path = package_to_asset_path(package)
        tokens.extend([package, asset_path, package.rsplit("/", 1)[-1]])
    tokens.extend(extra_tokens)
    tokens = [token for token in dict.fromkeys(tokens) if token]

    package_rows = []
    for package in packages:
        asset_path = package_to_asset_path(package)
        asset_rows = asset_data_for_package(all_assets, package)
        exists = bool(unreal.EditorAssetLibrary.does_asset_exist(asset_path))
        referencers = get_referencers(asset_path) if exists or asset_rows else []
        package_rows.append(
            {
                "package": package,
                "asset_path": asset_path,
                "exists": exists,
                "registry_assets": asset_rows,
                "referencers": referencers,
                "referencer_count": len(referencers) if isinstance(referencers, list) else None,
            }
        )

    report = {
        "project_root": str(PROJECT_ROOT),
        "packages": package_rows,
        "tokens": tokens,
        "metadata": metadata,
        "binary_content": scan_binary(tokens),
        "text": scan_text(tokens),
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    log(
        f"complete packages={len(package_rows)} "
        f"binary_matches={len(report['binary_content']['matches'])} "
        f"text_matches={len(report['text']['matches'])} output={output_path}"
    )
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        log("QUIT_EDITOR requested")
    except Exception as exc:
        warn(f"Failed to request QUIT_EDITOR: {exc}")


main()
