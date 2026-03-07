"""
Create M_PixelSprite -- a Niagara-compatible sprite material for square pixels.

Unlike the default Niagara sprite material (which has a circular alpha falloff
that makes particles look like round balls), this material outputs a solid
opaque square tinted by particle color. Used with NS_PixelParticle for the
retro pixel VFX look.

Translucent + Unlit + TwoSided.
  - Particle Color -> Emissive Color (tinted by User.Tint in Niagara)
  - Particle Alpha -> Opacity (fade-out over lifetime via Niagara Scale Color)

After running this script, open NS_PixelParticle in Niagara Editor:
  Sprite Renderer -> Material -> set to M_PixelSprite

Run in-editor: Tools > Execute Python Script > Scripts/CreatePixelSpriteMaterial.py
"""

import unreal

DEST_DIR = "/Game/VFX"
ASSET_NAME = "M_PixelSprite"
LOG_PREFIX = "[PixelSprite]"


def _ensure_dir(game_path):
    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(game_path):
            unreal.EditorAssetLibrary.make_directory(game_path)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} ensure_dir({game_path}): {e}")


def _safe_set(obj, prop, value):
    if obj is None:
        return
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        unreal.log_warning(f"{LOG_PREFIX} set_editor_property({prop}): {e}")


def create_pixel_sprite_material():
    path = f"{DEST_DIR}/{ASSET_NAME}"

    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log(f"{LOG_PREFIX} Deleting existing: {path}")
        unreal.EditorAssetLibrary.delete_asset(path)

    _ensure_dir(DEST_DIR)

    mel = unreal.MaterialEditingLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(ASSET_NAME, DEST_DIR, unreal.Material, factory)
    if not mat:
        unreal.log_error(f"{LOG_PREFIX} Failed to create {ASSET_NAME}")
        return False

    ok = True

    # Translucent + Unlit + TwoSided (required for Niagara sprites)
    try:
        _safe_set(mat, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        _safe_set(mat, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        _safe_set(mat, "two_sided", True)
        # Used with Niagara particles
        _safe_set(mat, "used_with_niagara_sprites", True)
        unreal.log(f"{LOG_PREFIX} Blend=Translucent, Shading=Unlit, TwoSided=True, NiagaraSprites=True")
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Material settings: {e}")
        ok = False

    # ParticleColor node -> Emissive Color
    # This picks up the per-particle color set by Niagara (driven by User.Tint)
    try:
        particle_color = mel.create_material_expression(
            mat, unreal.MaterialExpressionParticleColor, -300, 0)
        if particle_color:
            mel.connect_material_property(
                particle_color, "RGB",
                unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            mel.connect_material_property(
                particle_color, "A",
                unreal.MaterialProperty.MP_OPACITY)
            unreal.log(f"{LOG_PREFIX} ParticleColor.RGB -> EmissiveColor, .A -> Opacity")
        else:
            unreal.log_error(f"{LOG_PREFIX} ParticleColor node returned None")
            ok = False
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} ParticleColor: {e}")
        ok = False

    # Compile and save
    try:
        mel.recompile_material(mat)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Recompile: {e}")
        ok = False

    try:
        unreal.EditorAssetLibrary.save_asset(path)
    except Exception as e:
        unreal.log_error(f"{LOG_PREFIX} Save: {e}")
        ok = False

    if ok:
        unreal.log(f"{LOG_PREFIX} Created: {path}")
    return ok


def main():
    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} CreatePixelSpriteMaterial START")
    unreal.log(f"{'='*60}")

    result = create_pixel_sprite_material()

    unreal.log(f"{'='*60}")
    unreal.log(f"{LOG_PREFIX} DONE -- {'OK' if result else 'FAILED'}")
    unreal.log(f"{LOG_PREFIX}   Path: {DEST_DIR}/{ASSET_NAME}")
    unreal.log(f"{LOG_PREFIX}")
    unreal.log(f"{LOG_PREFIX} Next step: Open NS_PixelParticle in Niagara Editor")
    unreal.log(f"{LOG_PREFIX}   -> Sprite Renderer -> Material -> set to M_PixelSprite")
    unreal.log(f"{'='*60}")


if __name__ == "__main__":
    main()
