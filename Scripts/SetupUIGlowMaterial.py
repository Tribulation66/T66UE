import unreal

LOG = "[UIGlow]"
ASSET_DIR = "/Game/UI/Materials"
ASSET_NAME = "M_UI_Glow"
ASSET_PATH = f"{ASSET_DIR}/{ASSET_NAME}"

mel = unreal.MaterialEditingLibrary


def log(message):
    unreal.log(f"{LOG} {message}")


def safe_set(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
    except Exception as exc:
        unreal.log_warning(f"{LOG} failed to set {prop}: {exc}")


def connect(src, src_output, dst, dst_input):
    mel.connect_material_expressions(src, src_output, dst, dst_input)


def ensure_directory():
    if not unreal.EditorAssetLibrary.does_directory_exist(ASSET_DIR):
        unreal.EditorAssetLibrary.make_directory(ASSET_DIR)


def recreate_material():
    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        if not unreal.EditorAssetLibrary.delete_asset(ASSET_PATH):
            raise RuntimeError(f"Failed to delete existing asset {ASSET_PATH}")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = asset_tools.create_asset(ASSET_NAME, ASSET_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Failed to create material {ASSET_PATH}")
    return material


def build_material(material):
    safe_set(material, "material_domain", unreal.MaterialDomain.MD_UI)
    safe_set(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    safe_set(material, "two_sided", True)

    texcoord = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -1000, 0)

    center = mel.create_material_expression(material, unreal.MaterialExpressionConstant2Vector, -1000, 200)
    safe_set(center, "r", 0.5)
    safe_set(center, "g", 0.5)

    outer_mask = mel.create_material_expression(material, unreal.MaterialExpressionSphereMask, -720, -80)
    safe_set(outer_mask, "attenuation_radius", 0.72)
    safe_set(outer_mask, "hardness_percent", 18.0)

    inner_mask = mel.create_material_expression(material, unreal.MaterialExpressionSphereMask, -720, 170)
    safe_set(inner_mask, "attenuation_radius", 0.42)
    safe_set(inner_mask, "hardness_percent", 68.0)

    ring = mel.create_material_expression(material, unreal.MaterialExpressionSubtract, -440, 40)

    glow_color = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -430, -220)
    safe_set(glow_color, "parameter_name", "GlowColor")
    safe_set(glow_color, "default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    glow_intensity = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -430, 300)
    safe_set(glow_intensity, "parameter_name", "GlowIntensity")
    safe_set(glow_intensity, "default_value", 0.0)

    colored_ring = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -140, -20)
    final_color = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, 140, -20)
    final_opacity = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, 140, 200)

    connect(texcoord, "", outer_mask, "A")
    connect(center, "", outer_mask, "B")
    connect(texcoord, "", inner_mask, "A")
    connect(center, "", inner_mask, "B")

    connect(outer_mask, "", ring, "A")
    connect(inner_mask, "", ring, "B")

    connect(ring, "", colored_ring, "A")
    connect(glow_color, "", colored_ring, "B")

    connect(colored_ring, "", final_color, "A")
    connect(glow_intensity, "", final_color, "B")

    connect(ring, "", final_opacity, "A")
    connect(glow_intensity, "", final_opacity, "B")

    mel.connect_material_property(final_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(final_opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)


def main():
    ensure_directory()
    material = recreate_material()
    build_material(material)
    log(f"Created {ASSET_PATH}")


if __name__ == "__main__":
    main()
