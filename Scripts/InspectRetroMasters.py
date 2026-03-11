import unreal

OUT_PATH = unreal.Paths.project_saved_dir() + "RetroInspect.txt"
MATERIALS = [
    "/Game/Materials/M_Character_Unlit",
    "/Game/Materials/M_Environment_Unlit",
    "/Game/Materials/M_FBX_Unlit",
    "/Game/Materials/M_GLB_Unlit",
]

lines = []

def add(text=""):
    lines.append(str(text))
    unreal.log(str(text))

def dump_material(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        add(f"[RetroInspect] Missing: {path}")
        return
    add("=" * 80)
    add(f"[RetroInspect] Material: {path} ({asset.get_name()})")
    try:
        add(f"[RetroInspect] ShadingModel: {asset.get_editor_property('shading_model')}")
    except Exception as exc:
        add(f"[RetroInspect] ShadingModel read failed: {exc}")
    try:
        exprs = list(asset.get_editor_property('expressions') or [])
    except Exception as exc:
        add(f"[RetroInspect] Expression read failed: {exc}")
        exprs = []
    add(f"[RetroInspect] Expressions: {len(exprs)}")
    for expr in exprs:
        cls = type(expr).__name__
        name = ""
        try:
            name = expr.get_name()
        except Exception:
            pass
        desc = ""
        try:
            desc = expr.get_editor_property('desc')
        except Exception:
            pass
        extra = []
        for prop in ('parameter_name', 'material_function', 'texture'):
            try:
                value = expr.get_editor_property(prop)
                if value:
                    if hasattr(value, 'get_name'):
                        value = value.get_name()
                    extra.append(f"{prop}={value}")
            except Exception:
                pass
        add(f"  - {cls} name={name} desc={desc} {' '.join(extra)}")

for material_path in MATERIALS:
    dump_material(material_path)

with open(OUT_PATH, 'w', encoding='utf-8') as handle:
    handle.write("\n".join(lines))

add(f"[RetroInspect] Wrote {OUT_PATH}")
