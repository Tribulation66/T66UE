"""
Imports FBX character models (and their animations/textures if present) from:
  C:/UE/T66/SourceAssets/Extracted/
into Unreal Content paths:
  /Game/Characters/<Category>/<PackName>/

Category mapping:
  - Hero*      -> Heroes
  - Companion* -> Companions
  - Vendor     -> NPCs/Vendor
  - Gambler    -> NPCs/Gambler
  - SaintNPC   -> NPCs/Saint
  - RegularEnemy -> Enemies/Regular
  - GoblinThief  -> Enemies/GoblinThief
  - Leprachaun   -> Enemies/Leprechaun
  - StageBoss    -> Bosses/StageBoss

Run:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/ImportSourceAssetsModels.py"

Optional filter:
  -T66ImportPacks=PackA,PackB,PackC
"""

import os
import re
import unreal


PROJECT_SOURCE = r"C:\UE\T66\SourceAssets\Extracted"
QUIT_EDITOR_ON_FINISH = True


def try_set_editor_property(obj, prop: str, value) -> bool:
    """
    UE Python bindings can vary by engine version. Use this helper to set a property only when it exists.
    """
    if not obj:
        return False
    try:
        # Prefer editor-property API when available (best for UE reflected fields).
        if hasattr(obj, "has_editor_property") and obj.has_editor_property(prop):
            obj.set_editor_property(prop, value)
            return True
    except Exception:
        pass
    try:
        # Fallback: reflected properties can sometimes be accessible as attributes.
        if hasattr(obj, prop):
            setattr(obj, prop, value)
            return True
    except Exception:
        pass
    return False


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def list_fbx_files(root: str):
    out = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            if fn.lower().endswith(".fbx"):
                out.append(os.path.join(dirpath, fn))
    return out


def pick_primary_fbx(fbx_paths):
    """
    Prefer 'Character_output.fbx' as mesh, avoid pure animation-only files when possible.
    """
    if not fbx_paths:
        return None
    # Prefer mesh exports
    for p in fbx_paths:
        name = os.path.basename(p).lower()
        if "character_output" in name:
            return p
    # Avoid obvious animation-only
    for p in fbx_paths:
        name = os.path.basename(p).lower()
        if "animation" not in name:
            return p
    return fbx_paths[0]


def category_for_pack(pack_name: str) -> str:
    n = pack_name.lower()
    if n.startswith("hero"):
        return "Heroes"
    if n.startswith("companion"):
        return "Companions"
    if n == "vendor":
        return "NPCs/Vendor"
    if n == "gambler":
        return "NPCs/Gambler"
    if n == "saintnpc":
        return "NPCs/Saint"
    if n == "regularenemy":
        return "Enemies/Regular"
    if n == "goblinthief" or n.startswith("goblinthief_"):
        return "Enemies/GoblinThief"
    if n == "leprachaun" or n.startswith("leprachaun_") or n.startswith("leprechaun"):
        return "Enemies/Leprechaun"
    if n == "stageboss":
        return "Bosses/StageBoss"
    # Stage-specific packs (mobs + uniques) are stored together.
    if n.startswith("mob_stage") or n.startswith("unique_stage"):
        return "Enemies/Stages"
    # Boss visuals are keyed by BossID (Boss_01, Boss_02, ...)
    if re.match(r"^boss_\d+$", n):
        return "Bosses/Stages"
    return "Misc"


def make_dest_path(category: str, pack: str) -> str:
    return f"/Game/Characters/{category}/{pack}"


def import_fbx(
    file_path: str,
    dest_path: str,
    as_skeletal: bool,
    skeleton_asset_path: str | None,
    destination_name: str = "",
    import_mesh: bool = True,
    import_textures: bool = True,
    import_materials: bool = True,
    import_animations: bool = True,
):
    at = unreal.AssetToolsHelpers.get_asset_tools()

    task = unreal.AssetImportTask()
    task.automated = True
    try_set_editor_property(task, "suppress_import_dialogs", True)
    task.destination_name = destination_name or ""
    task.destination_path = dest_path
    task.filename = file_path
    task.replace_existing = True
    task.save = True

    opts = unreal.FbxImportUI()
    opts.import_mesh = import_mesh
    opts.import_textures = import_textures
    opts.import_materials = import_materials
    opts.import_as_skeletal = as_skeletal
    opts.import_animations = import_animations
    try_set_editor_property(opts, "create_physics_asset", False)

    # Help the importer choose the right path (property names vary by UE version).
    if as_skeletal:
        if import_mesh:
            try_set_editor_property(opts, "mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
        elif import_animations:
            try_set_editor_property(opts, "mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    else:
        # Static mesh import
        try_set_editor_property(opts, "mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)

    # Skeletal options
    if as_skeletal:
        # These fields are not stable across UE versions; set only if they exist.
        try_set_editor_property(opts.skeletal_mesh_import_data, "import_morph_targets", False)
        try_set_editor_property(opts.skeletal_mesh_import_data, "update_skeleton_reference_pose", False)
        try_set_editor_property(opts.skeletal_mesh_import_data, "use_t0_as_ref_pose", True)

        # If a skeleton path is provided and exists, force animations to bind to it.
        if skeleton_asset_path:
            skel = unreal.EditorAssetLibrary.load_asset(skeleton_asset_path)
            if skel:
                opts.skeleton = skel

    task.options = opts

    at.import_asset_tasks([task])
    return task.imported_object_paths


def find_first_skeleton_in_folder(dest_path: str):
    assets = unreal.EditorAssetLibrary.list_assets(dest_path, recursive=True, include_folder=False)
    for a in assets:
        obj = unreal.EditorAssetLibrary.load_asset(a)
        if obj and obj.get_class().get_name() == "Skeleton":
            return a
    return None


def _parse_pack_filter_from_cmdline() -> set[str]:
    """
    Supports:
      -T66ImportPacks=PackA,PackB,PackC
    If empty, imports all packs.
    """
    try:
        cmd = str(unreal.SystemLibrary.get_command_line())
    except Exception:
        cmd = ""

    # Grab up to first whitespace; allow quoted values too.
    m = re.search(r"-T66ImportPacks=(\"[^\"]+\"|[^\s]+)", cmd)
    if not m:
        return set()

    raw = m.group(1).strip()
    if raw.startswith("\"") and raw.endswith("\""):
        raw = raw[1:-1]

    parts = [p.strip() for p in re.split(r"[,\;]", raw) if p.strip()]
    return set(parts)


def main():
    unreal.log("=== ImportSourceAssetsModels ===")
    if not os.path.isdir(PROJECT_SOURCE):
        unreal.log_error("Missing: " + PROJECT_SOURCE)
        return

    packs = [d for d in os.listdir(PROJECT_SOURCE) if os.path.isdir(os.path.join(PROJECT_SOURCE, d))]
    pack_filter = _parse_pack_filter_from_cmdline()
    if pack_filter:
        packs = [p for p in packs if p in pack_filter]
        unreal.log(f"Pack filter active: importing {len(packs)} packs: {sorted(packs)}")
    if not packs:
        unreal.log_warning("No extracted packs found.")
        return

    for pack in sorted(packs):
        pack_dir = os.path.join(PROJECT_SOURCE, pack)
        fbx = list_fbx_files(pack_dir)
        if not fbx:
            unreal.log_warning(f"[{pack}] No FBX files found, skipping.")
            continue

        primary = pick_primary_fbx(fbx)
        category = category_for_pack(pack)
        dest = make_dest_path(category, pack)

        n = pack.lower()

        # Heuristic: most of these packs are skeletal characters.
        # Stage 2 Unique is allowed to be static (no animations).
        as_skeletal = True
        dest_name = ""
        if n == "unique_stage02":
            as_skeletal = False
            dest_name = "SM_Unique_Stage02"

        unreal.log(f"[{pack}] Importing primary FBX: {primary} -> {dest}")

        # First import the primary file to create skeletal mesh + skeleton.
        # Import the mesh without animations first. Many mesh FBXs contain junk/invalid embedded animation
        # ranges that can trigger Interchange FBX ensures; we import explicit animation FBXs separately.
        imported = import_fbx(
            primary,
            dest,
            as_skeletal=as_skeletal,
            skeleton_asset_path=None,
            destination_name=dest_name,
            import_animations=False,
        )
        unreal.log(f"[{pack}] Imported {len(imported)} assets from primary")

        # Find generated skeleton and use it for any additional animation FBXs in this pack.
        skel_path = find_first_skeleton_in_folder(dest)
        if skel_path:
            unreal.log(f"[{pack}] Using skeleton: {skel_path}")

        # Import remaining FBX files (often animations)
        if not as_skeletal:
            continue
        for other in fbx:
            if os.path.normcase(other) == os.path.normcase(primary):
                continue
            name = os.path.basename(other).lower()
            # Only import animation FBXs that look like animation exports to avoid duplicates.
            if "animation" in name or "walking" in name or "running" in name:
                unreal.log(f"[{pack}] Importing animation FBX: {other}")
                # Animation FBX should not generate duplicate skeletal meshes/materials.
                import_fbx(
                    other,
                    dest,
                    as_skeletal=True,
                    skeleton_asset_path=skel_path,
                    destination_name="",
                    import_mesh=False,
                    import_textures=False,
                    import_materials=False,
                    import_animations=True,
                )

    unreal.log("=== ImportSourceAssetsModels done ===")
    if QUIT_EDITOR_ON_FINISH:
        try:
            unreal.SystemLibrary.quit_editor()
        except Exception as e:
            unreal.log_warning(f"Failed to quit editor automatically: {e}")


if __name__ == "__main__":
    main()

