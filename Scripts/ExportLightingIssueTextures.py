import os

import unreal


EXPORT_DIR = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "LightingIssueTextures",
)

TEXTURE_TARGETS = [
    ("Rock", "/Game/World/Props/Rock.Rock", "BaseColorTexture"),
    ("Tree2", "/Game/World/Props/Tree2.Tree2", "BaseColorTexture"),
    ("BlackBag", "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black", "BaseColorTexture"),
]


def _get_texture_from_mesh(mesh_path, param_name):
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh:
        unreal.log_warning(f"[ExportLightingIssueTextures] Missing mesh: {mesh_path}")
        return None

    try:
        slots = mesh.get_editor_property("static_materials")
    except Exception:
        slots = []

    for slot in slots or []:
        try:
            material = slot.get_editor_property("material_interface")
        except Exception:
            material = None
        if not material:
            continue
        try:
            texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                material, param_name)
            if isinstance(texture, (tuple, list)):
                texture = texture[-1] if texture else None
        except Exception:
            texture = None
        if texture:
            return texture
    return None


def export_texture(label, mesh_path, param_name):
    texture = _get_texture_from_mesh(mesh_path, param_name)
    if not texture:
        unreal.log_warning(
            f"[ExportLightingIssueTextures] Missing {param_name} on {mesh_path}")
        return None

    os.makedirs(EXPORT_DIR, exist_ok=True)
    exporter = unreal.TextureExporterPNG()
    filename = os.path.join(EXPORT_DIR, f"{label}_{texture.get_name()}.png")

    task = unreal.AssetExportTask()
    task.object = texture
    task.filename = filename
    task.automated = True
    task.prompt = False
    task.exporter = exporter
    task.replace_identical = True

    exporter.run_asset_export_task(task)
    return filename


def main():
    exported = []
    for label, mesh_path, param_name in TEXTURE_TARGETS:
        filename = export_texture(label, mesh_path, param_name)
        if filename:
            exported.append(filename)
            unreal.log(
                f"[ExportLightingIssueTextures] Exported {mesh_path}:{param_name} -> {filename}")

    unreal.log(f"[ExportLightingIssueTextures] Exported {len(exported)} textures")


if __name__ == "__main__":
    main()
