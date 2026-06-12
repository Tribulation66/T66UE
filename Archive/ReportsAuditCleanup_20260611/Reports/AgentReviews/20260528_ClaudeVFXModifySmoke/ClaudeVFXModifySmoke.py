import hashlib
import json
import math
import traceback
from datetime import datetime, timezone
from pathlib import Path

import unreal


OUTPUT_DIR = Path(r"C:/UE/T66/Reports/AgentReviews/20260528_ClaudeVFXModifySmoke")
REPORT_PATH = OUTPUT_DIR / "vfx_modify_smoke_report.json"
PRODUCTION_SOURCE = "/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash"
LAB_SOURCE = "/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash"
TARGET_ASSET = "/Game/VFXLab/ClaudeSmoke/NS_Hero1AxeAOE_MeshSlash_ClaudeModifySmoke"
TARGET_DIR = "/Game/VFXLab/ClaudeSmoke"
TARGET_FILE = Path(r"C:/UE/T66/Content/VFXLab/ClaudeSmoke/NS_Hero1AxeAOE_MeshSlash_ClaudeModifySmoke.uasset")
FLOAT_TOLERANCE = 1e-6


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def log(message):
    unreal.log("[ClaudeVFXModifySmoke] " + str(message))


def fail(report, kind, message):
    report["success"] = False
    report["failure_kind"] = kind
    report.setdefault("errors", []).append(str(message))
    write_report(report)
    raise RuntimeError(f"{kind}: {message}")


def write_report(report):
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")


def load_report():
    if not REPORT_PATH.exists():
        return base_report()
    return json.loads(REPORT_PATH.read_text(encoding="utf-8"))


def base_report():
    return {
        "success": False,
        "failure_kind": None,
        "created_utc": utc_now(),
        "mode_history": [],
        "source_asset": {
            "path": "",
            "class_name": "",
            "exists": False,
        },
        "target_asset": {
            "path": TARGET_ASSET,
            "class_name": "",
            "exists": False,
        },
        "modified_property": {
            "name": "",
            "type": "",
            "before": None,
            "after_requested": None,
            "after_set": None,
            "after_verify": None,
            "tolerance": FLOAT_TOLERANCE,
        },
        "file_metadata": {
            "before_delete": file_metadata(),
            "after_delete": None,
            "after_save": None,
            "after_verify": None,
        },
        "save_success": False,
        "verify_process_success": False,
        "reload_verified": False,
        "errors": [],
    }


def file_metadata():
    if not TARGET_FILE.exists():
        return {
            "exists": False,
            "size": 0,
            "modified_utc": "",
            "sha256": "",
            "path": str(TARGET_FILE),
        }
    digest = hashlib.sha256(TARGET_FILE.read_bytes()).hexdigest()
    modified = datetime.fromtimestamp(TARGET_FILE.stat().st_mtime, timezone.utc).isoformat()
    return {
        "exists": True,
        "size": TARGET_FILE.stat().st_size,
        "modified_utc": modified,
        "sha256": digest,
        "path": str(TARGET_FILE),
    }


def get_command_mode():
    try:
        command_line = str(unreal.SystemLibrary.get_command_line())
    except Exception:
        command_line = ""
    marker = "-T66ClaudeVFXModifyMode="
    if marker not in command_line:
        return ""
    tail = command_line.split(marker, 1)[1]
    return tail.split()[0].strip().strip('"')


def class_name(asset):
    if asset is None:
        return ""
    try:
        return str(asset.get_class().get_name())
    except Exception:
        return ""


def choose_source_asset(report):
    for path in (PRODUCTION_SOURCE, LAB_SOURCE):
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            report["source_asset"] = {
                "path": path,
                "class_name": class_name(asset),
                "exists": True,
            }
            if asset is not None and class_name(asset) == "NiagaraSystem":
                return path
    fail(report, "MissingSourceNiagaraSystem", "No Hero 1 Axe AOE NiagaraSystem source asset was found.")


def ensure_target_scope():
    if not TARGET_ASSET.startswith("/Game/VFXLab/ClaudeSmoke/"):
        raise RuntimeError(f"Unsafe target scope: {TARGET_ASSET}")


def pick_mutation(asset, report):
    candidates = [
        ("warmup_time", "float", 0.125),
        ("warmup_tick_count", "int", 3),
        ("warmup_tick_delta", "float", 0.016666),
    ]
    failures = []
    for prop_name, prop_type, fallback_value in candidates:
        try:
            before = asset.get_editor_property(prop_name)
            if prop_type == "float":
                before_value = float(before)
                after_value = before_value + 0.125
                if abs(after_value - before_value) <= FLOAT_TOLERANCE:
                    after_value = fallback_value
                asset.set_editor_property(prop_name, after_value)
                after_set = float(asset.get_editor_property(prop_name))
            else:
                before_value = int(before)
                after_value = before_value + 1
                if after_value == before_value:
                    after_value = int(fallback_value)
                asset.set_editor_property(prop_name, after_value)
                after_set = int(asset.get_editor_property(prop_name))
            report["modified_property"] = {
                "name": prop_name,
                "type": prop_type,
                "before": before_value,
                "after_requested": after_value,
                "after_set": after_set,
                "after_verify": None,
                "tolerance": FLOAT_TOLERANCE,
            }
            if not values_match(after_value, after_set, prop_type):
                failures.append(f"{prop_name}: requested {after_value}, Unreal stored {after_set}")
                continue
            return
        except Exception as exc:
            failures.append(f"{prop_name}: {exc}")
    report["property_candidate_failures"] = failures
    fail(report, "NoWritableNiagaraProperty", "No candidate NiagaraSystem property could be read and set.")


def values_match(expected, actual, prop_type):
    if prop_type == "float":
        return math.isclose(float(expected), float(actual), rel_tol=0.0, abs_tol=FLOAT_TOLERANCE)
    return int(expected) == int(actual)


def modify_mode():
    report = base_report()
    report["mode_history"].append({"mode": "modify", "started_utc": utc_now()})
    ensure_target_scope()

    try:
        if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_DIR):
            unreal.EditorAssetLibrary.make_directory(TARGET_DIR)

        report["file_metadata"]["before_delete"] = file_metadata()
        if unreal.EditorAssetLibrary.does_asset_exist(TARGET_ASSET):
            if not unreal.EditorAssetLibrary.delete_asset(TARGET_ASSET):
                fail(report, "DeleteExistingTargetFailed", f"Could not delete previous smoke target: {TARGET_ASSET}")
        report["file_metadata"]["after_delete"] = file_metadata()

        source_path = choose_source_asset(report)
        duplicate = unreal.EditorAssetLibrary.duplicate_asset(source_path, TARGET_ASSET)
        if duplicate is None:
            fail(report, "DuplicateFailed", f"Duplicate failed from {source_path} to {TARGET_ASSET}")

        report["target_asset"] = {
            "path": TARGET_ASSET,
            "class_name": class_name(duplicate),
            "exists": True,
        }
        if class_name(duplicate) != "NiagaraSystem":
            fail(report, "TargetNotNiagaraSystem", f"Duplicate class was {class_name(duplicate)}")

        duplicate.modify()
        pick_mutation(duplicate, report)

        save_success = bool(unreal.EditorAssetLibrary.save_loaded_asset(duplicate))
        report["save_success"] = save_success
        report["file_metadata"]["after_save"] = file_metadata()
        if not save_success:
            fail(report, "SaveFailed", f"save_loaded_asset returned false for {TARGET_ASSET}")
        if not report["file_metadata"]["after_save"]["exists"]:
            fail(report, "SavedFileMissing", f"Target file missing after save: {TARGET_FILE}")
        if not report["file_metadata"]["after_save"]["sha256"]:
            fail(report, "SavedFileHashMissing", f"Target file hash missing after save: {TARGET_FILE}")

        report["failure_kind"] = "ModifyCompleted"
        write_report(report)
        log("Modify mode completed")
    except Exception:
        if not report.get("failure_kind") or report.get("failure_kind") == "ModifyCompleted":
            report["failure_kind"] = "ModifyException"
            report.setdefault("errors", []).append(traceback.format_exc())
            write_report(report)
        raise


def verify_mode():
    report = load_report()
    report.setdefault("mode_history", []).append({"mode": "verify", "started_utc": utc_now()})

    try:
        if not unreal.EditorAssetLibrary.does_asset_exist(TARGET_ASSET):
            fail(report, "VerifyTargetMissing", f"Target asset missing: {TARGET_ASSET}")
        target = unreal.EditorAssetLibrary.load_asset(TARGET_ASSET)
        if target is None:
            fail(report, "VerifyTargetLoadFailed", f"Target asset could not be loaded: {TARGET_ASSET}")
        report["target_asset"]["class_name"] = class_name(target)
        if class_name(target) != "NiagaraSystem":
            fail(report, "VerifyTargetNotNiagaraSystem", f"Target class was {class_name(target)}")

        prop = report["modified_property"]
        prop_name = prop.get("name", "")
        prop_type = prop.get("type", "")
        if not prop_name:
            fail(report, "MissingModifiedPropertyName", "No modified property name was recorded.")
        after_verify = target.get_editor_property(prop_name)
        after_verify = float(after_verify) if prop_type == "float" else int(after_verify)
        report["modified_property"]["after_verify"] = after_verify
        report["file_metadata"]["after_verify"] = file_metadata()

        if not values_match(prop["after_requested"], after_verify, prop_type):
            fail(report, "ReloadValueMismatch", f"{prop_name} expected {prop['after_requested']} after verify, got {after_verify}")
        if not report["file_metadata"]["after_verify"]["exists"]:
            fail(report, "VerifyFileMissing", f"Target file missing during verify: {TARGET_FILE}")
        if not report["file_metadata"]["after_verify"]["sha256"]:
            fail(report, "VerifyFileHashMissing", f"Target file hash missing during verify: {TARGET_FILE}")

        report["verify_process_success"] = True
        report["reload_verified"] = True
        report["success"] = True
        report["failure_kind"] = "Success"
        write_report(report)
        log("Verify mode completed")
    except Exception:
        if report.get("failure_kind") == "Success" or not report.get("failure_kind"):
            report["success"] = False
            report["failure_kind"] = "VerifyException"
            report.setdefault("errors", []).append(traceback.format_exc())
            write_report(report)
        raise


def main():
    mode = get_command_mode()
    if mode == "modify":
        modify_mode()
    elif mode == "verify":
        verify_mode()
    else:
        report = base_report()
        fail(report, "MissingMode", "Expected -T66ClaudeVFXModifyMode=modify or verify")


main()
