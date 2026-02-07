"""
Tune grass and landscape material instances to reduce "crunchy / sparkly" look:
- Lower normal strength (halve) so lighting is softer.
- Increase roughness so surfaces read matte, not glossy.

Target assets (T66MapAssets):
- /Game/T66MapAssets/Grass/MI_Plants_Grass  (Polytope grass)
- /Game/T66MapAssets/Landscape/MI_Landscape (Cozy Nature terrain)

Only parameters that exist on the base material are applied. If a parameter is missing,
the script logs it and you can add it in the base material or adjust manually in the MI.

Run in Unreal Editor: Tools -> Execute Python Script -> Scripts/TuneGrassAndLandscapeMaterials.py
"""

import unreal

# Asset paths (content paths)
GRASS_MI = "/Game/T66MapAssets/Grass/MI_Plants_Grass.MI_Plants_Grass"
LANDSCAPE_MI = "/Game/T66MapAssets/Landscape/MI_Landscape.MI_Landscape"

# Parameter names we try (common in Polytope/Cozy and many foliage materials)
NORMAL_STRENGTH_NAMES = ("NormalStrength", "Normal_Strength", "NormalStrengthMultiplier", "BumpStrength")
ROUGHNESS_NAMES = ("Roughness", "RoughnessMin", "Roughness_Min")

# Target values: softer normals, more matte
NORMAL_STRENGTH_TARGET = 0.5   # halve
ROUGHNESS_TARGET = 0.85       # matte


def set_scalar_if_present(mic, param_names, target_value, label):
    """Try to set one of the given parameter names to target_value. Returns True if any was set."""
    if not mic:
        return False
    for name in param_names:
        try:
            # Editor API: get current scalar overrides and see if this param exists
            scalars = mic.get_editor_property("scalar_parameter_values")
            if scalars is None:
                continue
            for i, entry in enumerate(scalars):
                try:
                    param_info = entry.get_editor_property("parameter_info") if hasattr(entry, "get_editor_property") else None
                    if param_info is None:
                        pname = entry.get_editor_property("parameter_name") if hasattr(entry, "get_editor_property") else None
                    else:
                        pname = param_info.get_editor_property("name") if hasattr(param_info, "get_editor_property") else None
                    if pname and pname == name:
                        val_prop = "parameter_value" if hasattr(entry, "get_editor_property") and "parameter_value" in dir(entry) else "value"
                        try:
                            entry.set_editor_property(val_prop, target_value)
                            unreal.log(f"[TuneGrassLandscape] {label}: set {name} = {target_value}")
                            mic.modify()
                            return True
                        except Exception:
                            pass
                        break
                except Exception:
                    continue
        except Exception as e:
            unreal.log(f"[TuneGrassLandscape] {label}: checking {name}: {e}")
    return False


def try_material_editing_library(mic, param_names, target_value, label):
    """Try MaterialEditingLibrary to set scalar on material instance (editor-only)."""
    try:
        mel = unreal.MaterialEditingLibrary
        for name in param_names:
            try:
                mel.set_material_instance_scalar_parameter_value(mic, name, target_value)
                unreal.log(f"[TuneGrassLandscape] {label}: set {name} = {target_value} (via MaterialEditingLibrary)")
                mic.modify()
                return True
            except Exception:
                continue
    except Exception as e:
        unreal.log(f"[TuneGrassLandscape] MaterialEditingLibrary: {e}")
    return False


def tune_mi(asset_path, label):
    """Load MI, apply normal strength and roughness tweaks, save if changed."""
    mic = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not mic:
        unreal.log_warning(f"[TuneGrassLandscape] Not found: {asset_path}")
        return False
    if not isinstance(mic, unreal.MaterialInstanceConstant):
        unreal.log_warning(f"[TuneGrassLandscape] Not a MaterialInstanceConstant: {asset_path}")
        return False

    changed = False
    if try_material_editing_library(mic, NORMAL_STRENGTH_NAMES, NORMAL_STRENGTH_TARGET, label):
        changed = True
    elif set_scalar_if_present(mic, NORMAL_STRENGTH_NAMES, NORMAL_STRENGTH_TARGET, label):
        changed = True
    else:
        unreal.log(f"[TuneGrassLandscape] {label}: no normal strength parameter found (tried {NORMAL_STRENGTH_NAMES}). Add one in base material or set manually.")

    if try_material_editing_library(mic, ROUGHNESS_NAMES, ROUGHNESS_TARGET, label):
        changed = True
    elif set_scalar_if_present(mic, ROUGHNESS_NAMES, ROUGHNESS_TARGET, label):
        changed = True
    else:
        unreal.log(f"[TuneGrassLandscape] {label}: no roughness parameter found (tried {ROUGHNESS_NAMES}). Set Roughness manually in MI if needed.")

    if changed:
        unreal.EditorAssetLibrary.save_asset(asset_path)
    return changed


def main():
    unreal.log("=== TuneGrassAndLandscapeMaterials START ===")
    tune_mi(GRASS_MI, "Grass")
    tune_mi(LANDSCAPE_MI, "Landscape")
    unreal.log("=== TuneGrassAndLandscapeMaterials DONE === See T66_Grass_Stylization_Plan.md for full fix (macro variation, blend variants).")


if __name__ == "__main__":
    main()
