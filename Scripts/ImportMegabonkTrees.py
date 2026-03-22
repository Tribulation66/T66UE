"""
Import Megabonk FBX tree meshes into the existing /Game/World/Props/Tree*
paths and bind them to a shared unlit material based on Tree.png.

Run inside Unreal Editor:
  py "C:/UE/T66/Scripts/ImportMegabonkTrees.py"
"""

import os
import unreal


LOG = "[ImportMegabonkTrees]"
PROJECT_MATERIAL = "/Game/Materials/M_Environment_Unlit"
DEST_DIR = "/Game/World/Props"
TEXTURE_NAME = "T_MegabonkTree"
MATERIAL_NAME = "MI_MegabonkTree"

IMPORTS = [
    ("MegabonkTrees/tree1.fbx", "Tree"),
    ("MegabonkTrees/tree2.fbx", "Tree2"),
    ("MegabonkTrees/tree3.fbx", "Tree3"),
]


def log(msg):
    unreal.log(f"{LOG} {msg}")


def warn(msg):
    unreal.log_warning(f"{LOG} {msg}")


def fail(msg):
    unreal.log_error(f"{LOG} {msg}")


def get_project_dir():
    try:
        return unreal.SystemLibrary.get_project_directory().replace("\\", "/").rstrip("/")
    except Exception:
        return os.path.abspath(os.path.join(os.getcwd(), "..")).replace("\\", "/")


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(source_path):
    ensure_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = TEXTURE_NAME

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{DEST_DIR}/{TEXTURE_NAME}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset or not isinstance(asset, unreal.Texture2D):
        raise RuntimeError(f"Texture import failed: {asset_path}")

    asset.set_editor_property("srgb", True)
    try:
        asset.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_World)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset


def ensure_tree_material(parent, texture):
    asset_path = f"{DEST_DIR}/{MATERIAL_NAME}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            MATERIAL_NAME,
            DEST_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
    if not asset:
        raise RuntimeError(f"Failed to create asset: {asset_path}")

    asset.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "DiffuseColorMap", texture)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(asset, "BaseColorTexture", texture)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        asset,
        "Tint",
        unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(asset, "Brightness", 1.0)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return asset


def cleanup_tree_asset(dest_name):
    asset_path = f"{DEST_DIR}/{dest_name}"
    subtree_path = f"{DEST_DIR}/{dest_name}"
    deleted_asset = False
    deleted_dir = False
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        deleted_asset = bool(unreal.EditorAssetLibrary.delete_asset(asset_path))
    if unreal.EditorAssetLibrary.does_directory_exist(subtree_path):
        deleted_dir = bool(unreal.EditorAssetLibrary.delete_directory(subtree_path))
    log(f"{dest_name}: cleanup deleted_asset={deleted_asset} deleted_dir={deleted_dir}")


def import_tree_mesh(source_path, dest_name):
    cleanup_tree_asset(dest_name)
    ensure_directory(DEST_DIR)

    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)

    static_mesh_data = options.get_editor_property("static_mesh_import_data")
    if static_mesh_data:
        try:
            static_mesh_data.set_editor_property("combine_meshes", False)
        except Exception:
            pass
        try:
            static_mesh_data.set_editor_property("generate_lightmap_u_vs", False)
        except Exception:
            pass
        try:
            static_mesh_data.set_editor_property("auto_generate_collision", True)
        except Exception:
            pass

    task = unreal.AssetImportTask()
    task.automated = True
    task.save = False
    task.replace_existing = True
    task.replace_existing_settings = True
    task.filename = source_path
    task.destination_path = DEST_DIR
    task.destination_name = dest_name
    task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh_path = f"{DEST_DIR}/{dest_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"StaticMesh import failed: {mesh_path}")
    unreal.EditorAssetLibrary.save_asset(mesh_path)
    return mesh


def assign_material_to_mesh(mesh, material):
    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        mesh.set_material(0, material)
    else:
        for index, slot in enumerate(slots):
            try:
                slot.set_editor_property("material_interface", material)
            except Exception:
                mesh.set_material(index, material)
        try:
            mesh.set_editor_property("static_materials", slots)
        except Exception:
            pass

    try:
        mesh.post_edit_change()
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_asset(mesh.get_path_name().split(".")[0])


def main():
    project_dir = get_project_dir()
    import_root = os.path.join(project_dir, "SourceAssets", "Import").replace("\\", "/")
    texture_source = os.path.join(import_root, "MegabonkTrees", "Tree.png").replace("\\", "/")
    if not os.path.isfile(texture_source):
        fail(f"Missing tree texture: {texture_source}")
        return

    parent = unreal.EditorAssetLibrary.load_asset(PROJECT_MATERIAL)
    if not parent:
        fail(f"Missing master material: {PROJECT_MATERIAL}")
        return

    log("START")
    texture = import_texture(texture_source)
    material = ensure_tree_material(parent, texture)
    log(f"Ready material: {material.get_path_name()}")

    for source_rel, dest_name in IMPORTS:
        source_path = os.path.join(import_root, source_rel).replace("\\", "/")
        if not os.path.isfile(source_path):
            warn(f"Missing FBX: {source_path}")
            continue

        log(f"Importing {source_rel} -> {DEST_DIR}/{dest_name}")
        mesh = import_tree_mesh(source_path, dest_name)
        assign_material_to_mesh(mesh, material)
        log(f"Ready mesh: {mesh.get_path_name()}")

    log("DONE")


if __name__ == "__main__":
    main()
