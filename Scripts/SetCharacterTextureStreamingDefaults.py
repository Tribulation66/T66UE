"""
Set imported character Texture2D assets to the character texture defaults.

Run in-editor:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/SetCharacterTextureStreamingDefaults.py"
"""

import json
import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import QuadRetroCharacterPipelineDefaults as CharacterDefaults


LOG_PREFIX = "[SetCharacterTextureStreamingDefaults]"
CHARACTER_ROOT = "/Game/Characters"
REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "SetCharacterTextureStreamingDefaults.json",
)
SAMPLE_TEXTURE_PATH = (
    "/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/"
    "Dungeon_Slime_QuadRetro/Textures/"
    "Dungeon_Slime_QuadRetro_Pixelated_512."
    "Dungeon_Slime_QuadRetro_Pixelated_512"
)


def _quit_editor():
    world = None
    try:
        subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if subsystem:
            world = subsystem.get_editor_world()
    except Exception:
        pass
    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        unreal.log_warning(f"{LOG_PREFIX} Failed to request QUIT_EDITOR: {exc}")


def _iter_character_textures():
    for asset_path in unreal.EditorAssetLibrary.list_assets(CHARACTER_ROOT, recursive=True, include_folder=False) or []:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset and isinstance(asset, unreal.Texture2D):
            yield asset_path, asset


def _texture_probe(texture):
    if not texture:
        return None
    try:
        texture.post_edit_change()
    except Exception:
        pass
    try:
        texture.update_resource()
    except Exception:
        pass
    payload = {
        "asset": texture.get_path_name(),
        "size_x": int(texture.blueprint_get_size_x()),
        "size_y": int(texture.blueprint_get_size_y()),
        "texture_streaming_method": CharacterDefaults.get_texture_streaming_method(texture),
        "lod_group": str(texture.get_editor_property("lod_group")),
        "max_texture_size": int(texture.get_editor_property("max_texture_size")),
        "lod_bias": int(texture.get_editor_property("lod_bias")),
        "compression_settings": str(texture.get_editor_property("compression_settings")),
        "never_stream": bool(texture.get_editor_property("never_stream")),
    }
    try:
        payload["is_possible_to_stream"] = bool(texture.is_possible_to_stream())
    except Exception:
        payload["is_possible_to_stream"] = None
    for prop_name in ("mip_gen_settings", "virtual_texture_streaming", "power_of_two_mode"):
        try:
            payload[prop_name] = str(texture.get_editor_property(prop_name))
        except Exception:
            pass
    return payload


def main():
    modified = []
    skipped = []
    errored = []

    sample_before = _texture_probe(unreal.EditorAssetLibrary.load_asset(SAMPLE_TEXTURE_PATH))

    for asset_path, texture in _iter_character_textures():
        try:
            result = CharacterDefaults.apply_character_texture_defaults(texture)
            if not result.get("ok"):
                errored.append({"asset": asset_path, "error": result.get("error", "defaults did not verify"), "result": result})
                unreal.log_error(f"{LOG_PREFIX} Failed {asset_path}: {result}")
                continue
            if result.get("changed"):
                saved = CharacterDefaults.safe_save(texture, asset_path)
                result["saved"] = saved
                modified.append(result)
                unreal.log(f"{LOG_PREFIX} Updated {asset_path}")
            else:
                skipped.append(result)
        except Exception as exc:
            errored.append({"asset": asset_path, "error": str(exc)})
            unreal.log_error(f"{LOG_PREFIX} Error {asset_path}: {exc}")

    sample_after = _texture_probe(unreal.EditorAssetLibrary.load_asset(SAMPLE_TEXTURE_PATH))

    payload = {
        "character_root": CHARACTER_ROOT,
        "modified_count": len(modified),
        "skipped_count": len(skipped),
        "errored_count": len(errored),
        "sample_texture": SAMPLE_TEXTURE_PATH,
        "sample_before": sample_before,
        "sample_after": sample_after,
        "modified": modified,
        "skipped": skipped,
        "errored": errored,
        "ok": len(errored) == 0
        and sample_after is not None
        and "CHARACTER" in sample_after["lod_group"].upper()
        and sample_after["never_stream"] is False
        and "NO_MIPMAPS" not in sample_after.get("mip_gen_settings", "").upper(),
    }

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, default=str)

    line = (
        f"{LOG_PREFIX} modified={len(modified)} skipped={len(skipped)} "
        f"errored={len(errored)} report={REPORT_PATH} ok={str(payload['ok']).lower()}"
    )
    if payload["ok"]:
        unreal.log(line)
    else:
        unreal.log_error(line)

    _quit_editor()


if __name__ == "__main__":
    main()
