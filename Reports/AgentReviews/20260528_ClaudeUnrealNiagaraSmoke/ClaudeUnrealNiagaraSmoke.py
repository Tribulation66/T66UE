import json
import traceback
from datetime import datetime, timezone
from pathlib import Path

import unreal


OUTPUT_DIR = Path(r"C:/UE/T66/Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke")
REPORT_PATH = OUTPUT_DIR / "unreal_niagara_smoke_report.json"
TARGET_ASSET = "/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash"
VFXLAB_PATH = "/Game/VFXLab"


def write_report(report):
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")


def get_engine_version():
    try:
        return str(unreal.SystemLibrary.get_engine_version())
    except Exception:
        return ""


def get_asset_class_name(asset_data):
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        try:
            return str(asset_data.asset_class)
        except Exception:
            return ""


def get_asset_package_name(asset_data):
    try:
        return str(asset_data.package_name)
    except Exception:
        try:
            return str(asset_data.get_editor_property("package_name"))
        except Exception:
            return ""


def get_asset_name(asset_data):
    try:
        return str(asset_data.asset_name)
    except Exception:
        try:
            return str(asset_data.get_editor_property("asset_name"))
        except Exception:
            return ""


def get_assets_by_path(registry, path):
    try:
        return list(registry.get_assets_by_path(unreal.Name(path), recursive=True))
    except TypeError:
        return list(registry.get_assets_by_path(unreal.Name(path), True))


def main():
    report = {
        "success": False,
        "failure_kind": None,
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "unreal_python_live": False,
        "engine_version": "",
        "target_asset": {
            "path": TARGET_ASSET,
            "exists": False,
            "loaded": False,
            "class_name": "",
            "class_path": "",
        },
        "niagara_api_symbol_count": 0,
        "niagara_api_symbols_sample": [],
        "vfxlab_asset_count": 0,
        "vfxlab_niagara_like_assets_sample": [],
        "errors": [],
    }

    try:
        report["unreal_python_live"] = True
        report["engine_version"] = get_engine_version()

        niagara_symbols = sorted([name for name in dir(unreal) if "Niagara" in name])
        report["niagara_api_symbol_count"] = len(niagara_symbols)
        report["niagara_api_symbols_sample"] = niagara_symbols[:80]

        asset_exists = bool(unreal.EditorAssetLibrary.does_asset_exist(TARGET_ASSET))
        report["target_asset"]["exists"] = asset_exists
        if asset_exists:
            asset = unreal.EditorAssetLibrary.load_asset(TARGET_ASSET)
            report["target_asset"]["loaded"] = asset is not None
            if asset is not None:
                asset_class = asset.get_class()
                report["target_asset"]["class_name"] = str(asset_class.get_name())
                report["target_asset"]["class_path"] = str(asset_class.get_path_name())

        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        vfxlab_assets = get_assets_by_path(registry, VFXLAB_PATH)
        report["vfxlab_asset_count"] = len(vfxlab_assets)

        niagara_like = []
        for asset_data in vfxlab_assets:
            class_name = get_asset_class_name(asset_data)
            asset_name = get_asset_name(asset_data)
            package_name = get_asset_package_name(asset_data)
            if "Niagara" in class_name or asset_name.startswith("NS_"):
                niagara_like.append(
                    {
                        "asset_name": asset_name,
                        "class_name": class_name,
                        "package_name": package_name,
                    }
                )
        report["vfxlab_niagara_like_assets_sample"] = niagara_like[:50]

        target_class = report["target_asset"]["class_name"]
        has_niagara_target = report["target_asset"]["loaded"] and "Niagara" in target_class
        has_niagara_api = report["niagara_api_symbol_count"] > 0
        has_niagara_registry_signal = len(niagara_like) > 0
        report["success"] = bool(
            report["unreal_python_live"]
            and report["engine_version"]
            and (has_niagara_target or has_niagara_api or has_niagara_registry_signal)
        )
        report["failure_kind"] = "Success" if report["success"] else "SmokeAssertionsFailed"
    except Exception as exc:
        report["failure_kind"] = "Exception"
        report["errors"].append(str(exc))
        report["traceback"] = traceback.format_exc()
    finally:
        write_report(report)

    if not report["success"]:
        raise RuntimeError(f"Claude Unreal/Niagara smoke failed: {report['failure_kind']}")

    unreal.log("[ClaudeUnrealNiagaraSmoke] Success. Report written to " + str(REPORT_PATH))


main()
