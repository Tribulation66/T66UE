"""Auto convex decomposition for solid interactable/gate meshes so blocking matches the
visible shape. Skips boost pickups (walk-through by design) and outline sidecars. Self-quits."""
from __future__ import annotations
import json, os
import unreal

OUT = r"C:/UE/T66/Saved/CombatTest/FallGuys/tighten_collision.json"
MESHES = [
    "/Game/World/Interactables/WeaponAltar/SM_WeaponAltar_Pixal3D",
    "/Game/World/Interactables/IdolAltar/SM_IdolAltar_Pixal3D",
    "/Game/World/Interactables/Chests/ChestModel/SM_Chest_Pixal3D",
    "/Game/World/Interactables/Crate",
    "/Game/World/Interactables/Crate/SM_LootCrate",
    "/Game/World/Interactables/Shroom",
    "/Game/World/Interactables/Shroom/Shroom_QuadRetro",
    "/Game/World/Interactables/Totem",
    "/Game/World/Interactables/DifficultyTotem/SM_DifficultyTotem_Pixal3D",
    "/Game/World/Interactables/Vending/Vending",
    "/Game/World/Interactables/Fountain/SM_Fountain_Pixal3D",
    "/Game/World/Interactables/LootWheel/SM_LootWheel_Pixal3D",
    "/Game/World/Interactables/CompanionCage/SM_CompanionCage_Pixal3D",
    "/Game/World/Interactables/Vehicles/SM_Vehicle_Pixal3D",
    "/Game/World/Gates/SM_CowardiceGate_Pixal3D",
    "/Game/World/Gates/SM_StageGate_Pixal3D",
    "/Game/World/Gates/SM_TutorialGate_Pixal3D",
    "/Game/World/Gates/TutorialGate_Pixal3D",
]
HULL_COUNT = 8
MAX_HULL_VERTS = 16
HULL_PRECISION = 100000
out = {"done": [], "errors": []}
def decompose(sms, mesh):
    """UE5.x renamed this API across versions — resolve whatever exists."""
    fn = getattr(sms, "set_convex_decomposition_collision", None)
    if fn:
        return bool(fn(mesh, HULL_COUNT, MAX_HULL_VERTS, HULL_PRECISION))
    fn = getattr(sms, "set_convex_decomposition_collisions", None)
    if fn:
        # 5.7: plural method name, singular static_mesh parameter.
        return bool(fn(mesh, HULL_COUNT, MAX_HULL_VERTS, HULL_PRECISION))
    fn = getattr(unreal, "EditorStaticMeshLibrary", None)
    if fn and hasattr(fn, "set_convex_decomposition_collision"):
        return bool(fn.set_convex_decomposition_collision(mesh, HULL_COUNT, MAX_HULL_VERTS, HULL_PRECISION))
    raise RuntimeError("no convex decomposition API found")

try:
    sms = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    out["api"] = [n for n in dir(sms) if "collision" in n.lower()]
    for path in MESHES:
        mesh = unreal.EditorAssetLibrary.load_asset(path)
        if not mesh:
            out["errors"].append("load failed: " + path)
            continue
        try:
            ok = decompose(sms, mesh)
        except Exception as exc:
            out["errors"].append("decomposition exception: %s: %s" % (path, exc))
            continue
        if not ok:
            out["errors"].append("decomposition failed: " + path)
            continue
        body = mesh.get_editor_property("body_setup")
        agg = body.get_editor_property("agg_geom")
        hulls = len(agg.get_editor_property("convex_elems"))
        saved = unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
        out["done"].append({"mesh": path, "hulls": hulls, "saved": bool(saved)})
finally:
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
    unreal.log("[TightenCollision] " + json.dumps(out))
    try: unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception: pass
