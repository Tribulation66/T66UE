"""
Batch-convert all remaining MaterialInstanceConstants to M_Environment_Unlit.

Scans the entire /Game/ content tree EXCEPT:
  - /Game/Characters/Heroes/, Companions/, Enemies/, NPCs/
  - /Game/Materials/
  - /Game/Lighting/
  - /Game/UI/

For each MIC: if not already M_Character_Unlit or M_Environment_Unlit,
read diffuse texture (or Tint color), reparent to M_Environment_Unlit,
set parameters, save.

Also logs Lit base Materials used by static meshes (need manual conversion).

Run INSIDE Unreal Editor:
  Tools -> Execute Python Script -> Scripts/ConvertEnvironmentMaterialsToUnlit.py

Run CreateEnvironmentUnlitMaterial.py FIRST to create M_Environment_Unlit.
"""

import unreal

ENV_UNLIT_PATH = "/Game/Materials/M_Environment_Unlit"
CHAR_UNLIT_PATH = "/Game/Materials/M_Character_Unlit"
LOG_PREFIX = "[EnvUnlitConvert]"

SKIP_DIRS = [
    "/Game/Characters/Heroes/",
    "/Game/Characters/Companions/",
    "/Game/Characters/Enemies/",
    "/Game/Characters/NPCs/",
    "/Game/Materials/",
    "/Game/Lighting/",
    "/Game/UI/",
]


def _path_under_skip(asset_path: str) -> bool:
    p = asset_path.replace("\\", "/")
    for d in SKIP_DIRS:
        if p.startswith(d) or ("/" + d.strip("/") + "/") in p or p == d.rstrip("/"):
            return True
    return False


def _asset_data_to_path(asset_data) -> str:
    """Convert AssetData to a path string suitable for load_asset."""
    try:
        return str(asset_data.object_path)
    except Exception:
        pass
    try:
        return str(asset_data.get_soft_object_path())
    except Exception:
        pass
    try:
        pkg = str(asset_data.package_name)
        name = str(asset_data.asset_name)
        return f"{pkg}.{name}"
    except Exception:
        return ""


def _is_already_unlit(mat_instance) -> bool:
    try:
        parent = mat_instance.get_editor_property("parent")
    except Exception:
        return False
    if not parent:
        return False
    try:
        path = parent.get_path_name()
        if path.startswith(CHAR_UNLIT_PATH) or path.startswith(ENV_UNLIT_PATH):
            return True
    except Exception:
        pass
    return False


def _get_first_texture_param(mat_instance, candidate_names):
    for param_name in candidate_names:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(
                mat_instance, param_name
            )
            if isinstance(result, (tuple, list)):
                tex = result[-1] if len(result) > 1 else result[0]
            else:
                tex = result
            if tex and isinstance(tex, unreal.Texture):
                return param_name, tex
        except Exception:
            pass
    return None, None


def _get_first_vector_param(mat_instance, candidate_names):
    for param_name in candidate_names:
        try:
            result = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(
                mat_instance, param_name
            )
            if result is not None:
                return param_name, result
        except Exception:
            pass
    return None, None


def _find_texture_in_directory(mat_path: str):
    package_path = mat_path.split(".")[0]
    dir_path = package_path.rsplit("/", 1)[0]
    try:
        asset_paths = unreal.EditorAssetLibrary.list_assets(
            dir_path, recursive=False, include_folder=False
        )
    except Exception:
        return None
    for p in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.Texture2D):
            return asset
    return None


def _convert_mic_to_environment_unlit(mat_instance, env_unlit_parent):
    mat_name = mat_instance.get_name()
    mat_path = mat_instance.get_path_name()

    if _is_already_unlit(mat_instance):
        return "skip_already_unlit", None

    # Read texture BEFORE reparenting
    diffuse_param_name, diffuse_tex = _get_first_texture_param(
        mat_instance, ["DiffuseColorMap", "BaseColorMap", "BaseColorTexture"]
    )
    source_desc = diffuse_param_name or ""

    if not diffuse_tex:
        diffuse_tex = _find_texture_in_directory(mat_path)
        if diffuse_tex:
            source_desc = f"(directory: {diffuse_tex.get_name()})"

    # Optional: read color for Tint (for solid-color materials)
    tint_param_name, tint_value = _get_first_vector_param(
        mat_instance, ["DiffuseColor", "BaseColor", "Color", "Tint"]
    )

    if not diffuse_tex and not tint_value:
        return "skip_no_texture_or_color", None

    if not diffuse_tex and tint_value:
        # Solid color only — set Tint, leave DiffuseColorMap as default
        try:
            mat_instance.set_editor_property("parent", env_unlit_parent)
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                mat_instance, "Tint", tint_value
            )
            unreal.EditorAssetLibrary.save_asset(mat_path)
            return "converted_tint_only", tint_param_name or "color"
        except Exception as e:
            return "error", str(e)

    # Has texture
    tex_name = diffuse_tex.get_name() if diffuse_tex else "None"
    try:
        mat_instance.set_editor_property("parent", env_unlit_parent)
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
            mat_instance, "DiffuseColorMap", diffuse_tex
        )
        if tint_value is not None:
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                mat_instance, "Tint", tint_value
            )
        unreal.EditorAssetLibrary.save_asset(mat_path)
        return "converted", f"texture={tex_name} (from {source_desc})"
    except Exception as e:
        return "error", str(e)


def _get_all_mic_paths():
    """Use Asset Registry to get only MaterialInstanceConstant paths, then filter by skip dirs."""
    paths = []
    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        asset_data_list = registry.get_assets_by_class("MaterialInstanceConstant", True)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} Asset Registry failed ({e}), falling back to list_assets")
        try:
            all_paths = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True, include_folder=False)
            for p in all_paths:
                if _path_under_skip(p):
                    continue
                paths.append(p)
        except Exception:
            pass
        return paths
    for asset_data in asset_data_list:
        try:
            p = _asset_data_to_path(asset_data)
            if not p or "/Game/" not in p:
                continue
            if _path_under_skip(p):
                continue
            paths.append(p)
        except Exception:
            pass
    return paths


def _find_lit_base_materials_on_static_meshes():
    """Scan StaticMesh assets; list base Materials (not MICs) that are Lit."""
    lit_materials = set()
    mesh_to_mats = {}
    try:
        mesh_paths = unreal.EditorAssetLibrary.list_assets(
            "/Game/", recursive=True, include_folder=False
        )
    except Exception:
        return lit_materials, mesh_to_mats

    for p in mesh_paths:
        if not p.endswith(".uasset"):
            continue
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if not asset or not isinstance(asset, unreal.StaticMesh):
            continue
        try:
            materials = asset.get_editor_property("static_materials")
        except Exception:
            try:
                materials = asset.get_editor_property("materials")
            except Exception:
                materials = []
        if not materials:
            continue
        for mat_slot in materials:
            try:
                mat_if = mat_slot.get_editor_property("material_interface")
            except Exception:
                mat_if = None
            if not mat_if:
                continue
            if isinstance(mat_if, unreal.MaterialInstanceConstant):
                continue
            if isinstance(mat_if, unreal.Material):
                try:
                    shading = mat_if.get_editor_property("shading_model")
                    if shading != unreal.MaterialShadingModel.MSM_UNLIT:
                        path = mat_if.get_path_name()
                        lit_materials.add(path)
                        if path not in mesh_to_mats:
                            mesh_to_mats[path] = []
                        mesh_to_mats[path].append(p)
                except Exception:
                    pass
    return lit_materials, mesh_to_mats


def main():
    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} ConvertEnvironmentMaterialsToUnlit START")
    unreal.log("=" * 60)

    env_unlit = unreal.EditorAssetLibrary.load_asset(ENV_UNLIT_PATH)
    if not env_unlit:
        unreal.log_error(
            f"{LOG_PREFIX} M_Environment_Unlit not found at {ENV_UNLIT_PATH}. "
            "Run CreateEnvironmentUnlitMaterial.py first!"
        )
        return

    unreal.log(f"{LOG_PREFIX} Loaded master: {ENV_UNLIT_PATH}")
    unreal.log(f"{LOG_PREFIX} Skipping dirs: {SKIP_DIRS}")

    # --- Collect all MIC paths under /Game/ (excluding skip dirs) ---
    all_candidate_paths = _get_all_mic_paths()
    mic_paths = []
    for p in all_candidate_paths:
        asset = unreal.EditorAssetLibrary.load_asset(p)
        if asset and isinstance(asset, unreal.MaterialInstanceConstant):
            mic_paths.append(p)

    unreal.log(f"{LOG_PREFIX} Found {len(mic_paths)} MaterialInstanceConstants to consider")

    converted = 0
    skipped_already = 0
    skipped_no_data = 0
    errors = 0
    tint_only = 0

    for p in mic_paths:
        mat = unreal.EditorAssetLibrary.load_asset(p)
        if not mat or not isinstance(mat, unreal.MaterialInstanceConstant):
            continue
        result, detail = _convert_mic_to_environment_unlit(mat, env_unlit)
        name = mat.get_name()
        if result == "converted":
            converted += 1
            unreal.log(f"  [CONVERTED] {name} — {detail}")
        elif result == "converted_tint_only":
            tint_only += 1
            unreal.log_warning(
                f"  [TINT-ONLY] {name} — no texture, set Tint from {detail}; review manually"
            )
        elif result == "skip_already_unlit":
            skipped_already += 1
            unreal.log(f"  [SKIP] {name} — already Unlit")
        elif result == "skip_no_texture_or_color":
            skipped_no_data += 1
            unreal.log_warning(f"  [SKIP] {name} — no diffuse texture or color param found")
        else:
            errors += 1
            unreal.log_error(f"  [ERROR] {name} — {detail}")

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log(
        f"{LOG_PREFIX} MICs: converted={converted}, tint_only={tint_only}, "
        f"skipped_already={skipped_already}, skipped_no_data={skipped_no_data}, errors={errors}"
    )

    # --- Task 3: Lit base Materials on static meshes ---
    unreal.log("")
    unreal.log(f"{LOG_PREFIX} Scanning for Lit base Materials on StaticMeshes ...")
    lit_mats, mesh_to_mats = _find_lit_base_materials_on_static_meshes()
    if lit_mats:
        unreal.log_warning(
            f"{LOG_PREFIX} Found {len(lit_mats)} base Material(s) (not MIC) still Lit — manual conversion needed:"
        )
        for mat_path in sorted(lit_mats):
            meshes = mesh_to_mats.get(mat_path, [])
            unreal.log_warning(f"  — {mat_path}")
            for m in meshes[:5]:
                unreal.log_warning(f"      used by: {m}")
            if len(meshes) > 5:
                unreal.log_warning(f"      ... and {len(meshes) - 5} more")
    else:
        unreal.log(f"{LOG_PREFIX} No Lit base Materials found on static meshes.")

    unreal.log("=" * 60)
    unreal.log(f"{LOG_PREFIX} DONE")
    unreal.log("=" * 60)


if __name__ == "__main__":
    main()
