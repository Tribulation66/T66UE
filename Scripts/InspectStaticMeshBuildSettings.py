import json
import os

import unreal


REPORT_PATH = os.path.join(
    unreal.SystemLibrary.get_project_directory(),
    "Saved",
    "Logs",
    "InspectStaticMeshBuildSettings.json",
)

ASSET_PATHS = [
    "/Game/World/Props/Tree2.Tree2",
    "/Game/World/Props/Rock.Rock",
    "/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black",
    "/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red",
    "/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow",
    "/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White",
]

BUILD_SETTING_PROPS = [
    "use_full_precision_u_vs",
    "use_backwards_compatible_f16_trunc_u_vs",
    "generate_lightmap_u_vs",
    "recompute_normals",
    "recompute_tangents",
]


def _inspect_mesh(asset_path):
    mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
    info = {
        "path": asset_path,
        "loaded": bool(mesh),
    }
    if not mesh:
        return info

    info["name"] = mesh.get_name()
    info["class"] = type(mesh).__name__
    lods = []

    try:
        lod_count = int(mesh.get_num_lods())
        info["lod_count_via_mesh"] = lod_count
    except Exception as exc:
        info["lod_count_via_mesh_error"] = str(exc)
        lod_count = 0

    if lod_count <= 0:
        try:
            subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
            lod_count = int(subsystem.get_lod_count(mesh))
            info["lod_count_via_subsystem"] = lod_count
        except Exception as exc:
            info["lod_count_via_subsystem_error"] = str(exc)

    if lod_count <= 0:
        try:
            lod_count = int(unreal.EditorStaticMeshLibrary.get_lod_count(mesh))
            info["lod_count_via_library"] = lod_count
        except Exception as exc:
            info["lod_count_via_library_error"] = str(exc)

    for index in range(lod_count):
        lod_info = {"lod_index": index}
        try:
            subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
            build_settings = subsystem.get_lod_build_settings(mesh, index)
        except Exception as exc:
            lod_info["build_settings_via_subsystem_error"] = str(exc)
            try:
                build_settings = unreal.EditorStaticMeshLibrary.get_lod_build_settings(mesh, index)
            except Exception as inner_exc:
                lod_info["build_settings_via_library_error"] = str(inner_exc)
                lods.append(lod_info)
                continue

        for prop_name in BUILD_SETTING_PROPS:
            try:
                lod_info[prop_name] = build_settings.get_editor_property(prop_name)
            except Exception as exc:
                lod_info[prop_name] = f"ERROR: {exc}"

        lods.append(lod_info)

    info["lods"] = lods
    return info


def main():
    report = {
        "assets": [_inspect_mesh(path) for path in ASSET_PATHS],
    }

    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"[InspectStaticMeshBuildSettings] Wrote {REPORT_PATH}")


if __name__ == "__main__":
    main()
