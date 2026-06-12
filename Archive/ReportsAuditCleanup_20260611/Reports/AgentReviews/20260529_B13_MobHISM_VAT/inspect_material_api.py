import json
import os

import unreal


PROJECT_ROOT = r"C:\UE\T66"
OUT_PATH = os.path.join(PROJECT_ROOT, "Saved", "Codex", "Performance", "B13_MobHISM", "material_api_probe.json")

paths = [
    "/Game/Materials/M_EasyMobVAT_Unlit_UV2",
    "/Game/Materials/M_Character_Unlit",
]

out = {
    "MaterialEditingLibrary": [name for name in dir(unreal.MaterialEditingLibrary) if "expression" in name.lower() or "material" in name.lower()],
    "assets": {},
}

for path in paths:
    asset = unreal.EditorAssetLibrary.load_asset(path)
    entry = {
        "exists": bool(asset),
        "class": asset.get_class().get_name() if asset else "",
        "dir_subset": [],
        "properties": [],
    }
    if asset:
        entry["dir_subset"] = [name for name in dir(asset) if "express" in name.lower() or "editor" in name.lower() or "material" in name.lower()]
        try:
            entry["properties"] = [p.get_name() for p in asset.get_class().get_properties()]
        except Exception as exc:
            entry["properties_error"] = str(exc)
        for prop in ("expressions", "editor_only_data", "material_editor_instance_constant", "blend_mode", "shading_model"):
            try:
                value = asset.get_editor_property(prop)
                entry[prop] = str(value)
                if prop == "editor_only_data" and value:
                    entry["editor_only_data_dir_subset"] = [name for name in dir(value) if "express" in name.lower() or "collection" in name.lower()]
                    try:
                        entry["editor_only_data_properties"] = [p.get_name() for p in value.get_class().get_properties()]
                    except Exception as exc:
                        entry["editor_only_data_properties_error"] = str(exc)
            except Exception as exc:
                entry[prop + "_error"] = str(exc)
    out["assets"][path] = entry

os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
with open(OUT_PATH, "w", encoding="utf-8") as handle:
    json.dump(out, handle, indent=2)

unreal.log("B13 material API probe wrote {}".format(OUT_PATH))
