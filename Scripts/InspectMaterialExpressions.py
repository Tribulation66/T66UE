import unreal


MATERIAL_PATH = "/Game/UE5RFX/Materials/BaseMaterials/UE5RFX_Flipbook_BaseMaterial"

material = unreal.load_asset(MATERIAL_PATH)
if not material:
    raise RuntimeError(f"Missing {MATERIAL_PATH}")

unreal.log(f"Material: {MATERIAL_PATH}")
expressions = list(material.get_editor_property("expressions"))
for expr in expressions:
    cls = expr.get_class().get_name()
    desc = ""
    try:
        if hasattr(expr, "parameter_name"):
            desc = f" parameter_name={expr.get_editor_property('parameter_name')}"
    except Exception:
        pass
    unreal.log(f"  {cls}{desc}")
