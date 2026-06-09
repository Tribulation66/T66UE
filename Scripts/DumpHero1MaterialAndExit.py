"""Throwaway runtime dump: report the material setup Hero_1_Chad actually resolves to.

Loads the DT_CharacterVisuals Hero_1_Chad row, loads the referenced skeletal mesh,
and dumps each material slot's interface path / class / base material / referenced
textures. This is the embedded material the runtime DMI is built from
(T66ApplySafeCharacterMaterialOverrides). Writes JSON to Saved/CombatTest and self-quits.
"""
from __future__ import annotations

import csv
import io
import json
from pathlib import Path

import unreal

DT_PATH = "/Game/Data/DT_CharacterVisuals"
ROW = "Hero_1_Chad"


def class_name(obj) -> str:
    try:
        return obj.get_class().get_name()
    except Exception:
        return "<unknown>"


def main() -> int:
    out = {"row": ROW, "errors": [], "skeletal_mesh": None, "materials": []}

    sk_path = ""
    dt = unreal.EditorAssetLibrary.load_asset(DT_PATH)
    if dt:
        exported = unreal.DataTableFunctionLibrary.export_data_table_to_csv_string(dt)
        for parsed in csv.DictReader(io.StringIO(exported)):
            if parsed.get("---", "") == ROW:
                sk_path = parsed.get("SkeletalMesh", "") or ""
                break
    else:
        out["errors"].append(f"missing DataTable: {DT_PATH}")
    out["skeletal_mesh_path_from_datatable"] = sk_path

    if not sk_path:
        out["errors"].append("could not resolve SkeletalMesh path from data table; using known fallback")
        sk_path = "/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst"

    sk = unreal.EditorAssetLibrary.load_asset(sk_path.split(".")[0])
    if not sk:
        out["errors"].append(f"could not load skeletal mesh: {sk_path}")
    else:
        out["skeletal_mesh"] = sk.get_path_name()
        try:
            mats = sk.get_editor_property("materials")
        except Exception as exc:
            mats = []
            out["errors"].append(f"could not read materials array: {exc}")
        for i, m in enumerate(mats):
            entry = {"slot_index": i}
            try:
                entry["slot_name"] = str(m.get_editor_property("material_slot_name"))
            except Exception:
                entry["slot_name"] = ""
            mi = None
            try:
                mi = m.get_editor_property("material_interface")
            except Exception as exc:
                entry["error"] = f"no material_interface: {exc}"
            if mi:
                entry["material_path"] = mi.get_path_name()
                entry["material_class"] = class_name(mi)
                try:
                    base = mi.get_base_material()
                    entry["base_material_path"] = base.get_path_name() if base else ""
                except Exception:
                    entry["base_material_path"] = ""
                try:
                    ar = unreal.AssetRegistryHelpers.get_asset_registry()
                    pkg = mi.get_outermost().get_path_name()
                    opts = unreal.AssetRegistryDependencyOptions(include_hard_package_references=True)
                    deps = ar.get_dependencies(unreal.Name(pkg), opts) or []
                    entry["package_dependencies"] = [str(d) for d in deps]
                except Exception as exc:
                    entry["dep_error"] = str(exc)
            else:
                entry["material_path"] = ""
            out["materials"].append(entry)

    report = Path(unreal.SystemLibrary.get_project_directory()) / "Saved" / "CombatTest" / "Hero1_Material_RuntimeDump.json"
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(out, indent=2), encoding="utf-8")
    unreal.log(f"[DumpHero1Material] wrote {report}")
    unreal.log(f"[DumpHero1Material] RESULT {json.dumps(out)}")
    return 0


try:
    main()
finally:
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
    except Exception:
        pass
