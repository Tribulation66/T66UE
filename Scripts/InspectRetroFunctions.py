import unreal

ASSETS = [
    "/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexSnapping",
    "/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_VertexNoise",
    "/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_UV_AffineMapping",
    "/Game/UE5RFX/Materials/MaterialFunctions/UE5RFX_WPO_Fake_Sprite_Snap",
    "/Game/UE5RFX/Materials/UE5RFX_MaterialParameterCollection",
]


def log(text):
    unreal.log("[RetroInspectFns] " + str(text))


for path in ASSETS:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        log(f"Missing asset: {path}")
        continue

    log("=" * 80)
    log(f"Asset: {path}")
    log(f"Type: {type(asset).__name__}")

    for prop in ("function_expressions", "scalar_parameters", "vector_parameters"):
        try:
            value = asset.get_editor_property(prop)
            if value is None:
                continue
            log(f"{prop}: {len(value) if hasattr(value, '__len__') else value}")
            if hasattr(value, "__iter__"):
                for item in value:
                    try:
                        log(f"  - {type(item).__name__} {item.get_name()}")
                    except Exception:
                        log(f"  - {item}")
        except Exception as exc:
            log(f"{prop} unavailable: {exc}")
