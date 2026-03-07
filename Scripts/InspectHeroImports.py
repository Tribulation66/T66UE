import os
import unreal


LINES = []


def inspect_folder(path):
    LINES.append(f"== {path} ==")
    assets = unreal.EditorAssetLibrary.list_assets(path, recursive=False, include_folder=False)
    for asset_path in assets:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        class_name = asset.get_class().get_name() if asset else "None"
        LINES.append(f"{asset_path} :: {class_name}")


def main():
    for idx in (1, 2, 16):
        inspect_folder(f"/Game/Characters/Heroes/Hero_{idx}/TypeA")
    proj = unreal.SystemLibrary.get_project_directory()
    out_path = os.path.join(proj, "Saved", "InspectHeroImports.txt")
    with open(out_path, "w", encoding="utf-8") as handle:
        handle.write("\n".join(LINES))


if __name__ == "__main__":
    main()
