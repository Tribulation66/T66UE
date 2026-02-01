"""
Asset usage audit for T66 (focus: UI Blueprints / WBP_*).

Run from Unreal Editor:
  Py Execute Script -> Scripts/AssetAudit.py

Or via commandlet:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/AssetAudit.py"
"""

import os
import datetime
import unreal


def project_root() -> str:
    return unreal.SystemLibrary.get_project_directory()


def write_report(text: str, rel_path: str) -> str:
    out_path = os.path.normpath(os.path.join(project_root(), rel_path))
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(text)
    return out_path


def get_asset_registry():
    return unreal.AssetRegistryHelpers.get_asset_registry()


def get_referencers(ar, package_name: str):
    """
    Returns a list of referencer package names (strings) for a given package.
    Includes hard + soft references.
    """
    # Unreal's Python bindings for AssetRegistryDependencyOptions differ across versions
    # (constructor keywords and even field names vary). Build options defensively.
    opts = unreal.AssetRegistryDependencyOptions()
    for attr, value in (
        # C++: bIncludeHardPackageReferences / bIncludeSoftPackageReferences
        ("include_hard_package_references", True),
        ("include_soft_package_references", True),
        # C++: bIncludeSearchableNames
        ("include_searchable_names", False),
        # C++: bIncludeHardManagementReferences / bIncludeSoftManagementReferences
        ("include_hard_management_references", False),
        ("include_soft_management_references", False),
    ):
        try:
            setattr(opts, attr, value)
        except Exception:
            pass
    try:
        # UE5: get_referencers(package_name, options)
        return ar.get_referencers(package_name, opts)
    except TypeError:
        # Older signature: get_referencers(package_name, options, recursive)
        try:
            return ar.get_referencers(package_name, opts, True)
        except Exception:
            return []


def is_game_package(pkg: str) -> bool:
    return isinstance(pkg, str) and pkg.startswith("/Game/")


def main():
    unreal.log("=== T66 AssetAudit: starting ===")

    ar = get_asset_registry()
    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    ui_root = "/Game/Blueprints/UI"
    assets = ar.get_assets_by_path(ui_root, recursive=True)

    # Focus: WBP_* WidgetBlueprints and other UI Blueprints.
    wbps = []
    for a in assets:
        name = str(a.asset_name)
        if not name.startswith("WBP_"):
            continue
        # asset_class_path is preferred in UE5; fall back to asset_class string.
        cls = ""
        try:
            cls = str(a.asset_class_path.asset_name)
        except Exception:
            cls = str(a.asset_class)
        # Avoid duplicate rows for the same package (e.g., WidgetBlueprintGeneratedClass).
        if cls != "WidgetBlueprint":
            continue
        pkg = str(a.package_name)

        # AssetData API varies a bit across UE/Python contexts (Editor vs commandlet).
        # We only need a stable string for reporting, not for loading.
        obj_path = ""
        try:
            obj_path = str(a.object_path)  # UE versions where this exists
        except Exception:
            try:
                obj_path = str(a.get_soft_object_path())  # preferred when available
            except Exception:
                # Fallback: standard Unreal object path format for assets.
                obj_path = f"{pkg}.{name}"

        wbps.append((name, cls, pkg, obj_path))

    # Known optional-by-design overrides (C++ fallback exists).
    optional_overrides = {
        "WBP_LanguageSelect",
        "WBP_Achievements",
        "WBP_SaveSlots",
    }

    rows = []
    unreferenced = []
    unreferenced_optional = []
    referenced = []

    for (name, cls, pkg, obj_path) in sorted(wbps, key=lambda t: t[0].lower()):
        refs_raw = get_referencers(ar, pkg)
        refs = []
        for r in refs_raw:
            rs = str(r)
            if is_game_package(rs) and rs != pkg:
                refs.append(rs)
        refs_sorted = sorted(set(refs))

        entry = {
            "name": name,
            "class": cls,
            "package": pkg,
            "object_path": obj_path,
            "referencers": refs_sorted,
            "referencer_count": len(refs_sorted),
            "is_optional_override": name in optional_overrides,
        }
        rows.append(entry)

        if entry["referencer_count"] <= 0:
            if entry["is_optional_override"]:
                unreferenced_optional.append(entry)
            else:
                unreferenced.append(entry)
        else:
            referenced.append(entry)

    lines = []
    lines.append("# T66 Asset Audit Report")
    lines.append(f"Generated: {now}")
    lines.append("")
    lines.append("Scope:")
    lines.append(f"- Path: `{ui_root}`")
    lines.append("- Filter: `WBP_*` assets")
    lines.append("- Reference types: hard + soft referencers from AssetRegistry (limited to `/Game/**`)")
    lines.append("")
    lines.append("## Summary")
    lines.append(f"- Total `WBP_*` found: **{len(rows)}**")
    lines.append(f"- Referenced: **{len(referenced)}**")
    lines.append(f"- Unreferenced (non-optional): **{len(unreferenced)}**")
    lines.append(f"- Unreferenced (optional overrides): **{len(unreferenced_optional)}**")
    lines.append("")

    def render_list(title: str, items):
        lines.append(f"## {title}")
        if not items:
            lines.append("- (none)")
            lines.append("")
            return
        for e in items:
            lines.append(f"- `{e['package']}` ({e['class']})")
        lines.append("")

    render_list("Unreferenced (non-optional candidates)", unreferenced)
    render_list("Unreferenced (optional-by-design overrides)", unreferenced_optional)

    lines.append("## Referencers (details)")
    for e in rows:
        lines.append(f"### `{e['package']}`")
        lines.append(f"- **Class**: `{e['class']}`")
        lines.append(f"- **ObjectPath**: `{e['object_path']}`")
        lines.append(f"- **OptionalOverride**: `{e['is_optional_override']}`")
        if e["referencer_count"] <= 0:
            lines.append("- **Referencers**: (none under `/Game/**`)")
        else:
            lines.append("- **Referencers**:")
            for r in e["referencers"]:
                lines.append(f"  - `{r}`")
        lines.append("")

    report_rel = "T66_Asset_Audit_Report.md"
    report_path = write_report("\n".join(lines) + "\n", report_rel)
    unreal.log(f"AssetAudit: wrote report to: {report_path}")
    unreal.log("=== T66 AssetAudit: done ===")


if __name__ == "__main__":
    main()

