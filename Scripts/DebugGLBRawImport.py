import json
import os

import unreal


IMPORTS = [
    ("Props/Tree2.glb", "/Game/__Debug/GLBRaw/Tree2", "Tree2Raw"),
    ("Props/Rock.glb", "/Game/__Debug/GLBRaw/Rock", "RockRaw"),
]

REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "DebugGLBRawImport.json",
)

EXPORT_DIR = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "DebugGLBRawTextures",
)

KNOWN_TEXTURE_PARAMS = [
    "BaseColorTexture",
    "DiffuseColorMap",
    "MetallicRoughnessTexture",
    "NormalTexture",
    "NormalMap",
    "OcclusionTexture",
    "EmissiveTexture",
    "OpacityTexture",
]


def _project_import_root():
    return os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "SourceAssets",
        "Import",
    ).replace("\\", "/")


def _asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return None


def _texture_info(texture):
    if not texture:
        return None
    info = {
        "path": _asset_path(texture),
        "name": texture.get_name(),
        "class": texture.get_class().get_name(),
    }
    for prop in [
        "srgb",
        "compression_settings",
        "lod_group",
        "mip_gen_settings",
        "filter",
        "never_stream",
    ]:
        try:
            info[prop] = str(texture.get_editor_property(prop))
        except Exception:
            pass
    for name, getter in [("size_x", "blueprint_get_size_x"), ("size_y", "blueprint_get_size_y")]:
        try:
            info[name] = int(getattr(texture, getter)())
        except Exception:
            pass
    return info


def _inspect_material_instance(asset):
    info = {
        "path": _asset_path(asset),
        "name": asset.get_name(),
        "class": asset.get_class().get_name(),
    }
    try:
        parent = asset.get_editor_property("parent")
        info["parent"] = _asset_path(parent)
    except Exception:
        pass

    textures = {}
    for param_name in KNOWN_TEXTURE_PARAMS:
        try:
            tex = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                asset, param_name)
            if isinstance(tex, (tuple, list)):
                tex = tex[-1] if tex else None
        except Exception:
            tex = None
        if tex:
            textures[param_name] = _texture_info(tex)
    if textures:
        info["texture_parameters"] = textures
    return info


def _export_texture(texture, prefix):
    if not texture:
        return None
    os.makedirs(EXPORT_DIR, exist_ok=True)
    filename = os.path.join(EXPORT_DIR, f"{prefix}_{texture.get_name()}.png")
    task = unreal.AssetExportTask()
    task.object = texture
    task.filename = filename
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    task.exporter = unreal.TextureExporterPNG()
    try:
        task.exporter.run_asset_export_task(task)
        return filename
    except Exception as exc:
        unreal.log_warning(f"[DebugGLBRawImport] Failed to export {texture.get_name()}: {exc}")
        return None


def _delete_debug_root():
    root = "/Game/__Debug/GLBRaw"
    try:
        if unreal.EditorAssetLibrary.does_directory_exist(root):
            unreal.EditorAssetLibrary.delete_directory(root)
    except Exception as exc:
        unreal.log_warning(f"[DebugGLBRawImport] Failed to clear {root}: {exc}")


def _import_glb(source_path, dest_dir, dest_name):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest_dir):
        unreal.EditorAssetLibrary.make_directory(dest_dir)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = dest_dir
    task.destination_name = dest_name

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths or [])


def _list_assets(root):
    try:
        return unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
    except Exception:
        return []


def main():
    _delete_debug_root()
    report = {"imports": []}

    import_root = _project_import_root()
    unreal.log("[DebugGLBRawImport] START")

    for relative_path, dest_dir, dest_name in IMPORTS:
        source_path = os.path.join(import_root, relative_path).replace("\\", "/")
        row = {
            "source": source_path,
            "dest_dir": dest_dir,
            "dest_name": dest_name,
        }
        imported = _import_glb(source_path, dest_dir, dest_name)
        row["imported_object_paths"] = imported

        assets = []
        for asset_path in _list_assets(dest_dir):
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset:
                assets.append({"path": asset_path, "missing": True})
                continue

            entry = {
                "path": asset_path,
                "name": asset.get_name(),
                "class": asset.get_class().get_name(),
            }
            if isinstance(asset, unreal.MaterialInstanceConstant):
                entry.update(_inspect_material_instance(asset))
                texture_exports = {}
                for param_name, info in (entry.get("texture_parameters") or {}).items():
                    tex = unreal.EditorAssetLibrary.load_asset(info["path"])
                    export_path = _export_texture(tex, f"{dest_name}_{param_name}")
                    if export_path:
                        texture_exports[param_name] = export_path
                if texture_exports:
                    entry["texture_exports"] = texture_exports
            assets.append(entry)

        row["assets"] = assets
        report["imports"].append(row)

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[DebugGLBRawImport] Wrote {REPORT_PATH}")
    unreal.log("[DebugGLBRawImport] DONE")


if __name__ == "__main__":
    main()
