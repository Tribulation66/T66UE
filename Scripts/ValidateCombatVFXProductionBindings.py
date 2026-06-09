"""
Validate the production Combat VFX binding path.

Run full Unreal validation with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/ValidateCombatVFXProductionBindings.py" -unattended -nop4 -nosplash

Run parser self-test without Unreal:
  python C:/UE/T66/Scripts/ValidateCombatVFXProductionBindings.py --self-test-root C:/UE/T66/Saved/Tmp/CombatVFXValidatorSelfTest
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path


LOG = "[CombatVFXProductionBindingsValidate]"
DISCLAIMER = (
    "This validator proves Combat VFX binding structure, required assets, source guards, "
    "and declared data contracts. It does not prove visual fidelity, temporal mechanism "
    "quality, final player-facing readability, or Pablo visual approval."
)
PROJECT_ROOT = Path(r"C:/UE/T66")
CSV_PATH = PROJECT_ROOT / "Content" / "Data" / "CombatVFXBindings.csv"
WEAPONS_CSV_PATH = PROJECT_ROOT / "Content" / "Data" / "Weapons.csv"
DATA_TYPES_H = PROJECT_ROOT / "Source" / "T66" / "Data" / "T66DataTypes.h"
COMBAT_COMPONENT_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66CombatComponent.cpp"
COMBAT_SHARED_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66CombatShared.cpp"
OVERLAYS_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66PlayerController_Overlays.cpp"
CAPTURE_SCRIPT = PROJECT_ROOT / "Scripts" / "CaptureT66GameplayVideo.ps1"
BP_GAME_INSTANCE_CLASS_PATH = "/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C"

DT_ASSET = "/Game/Data/DT_CombatVFXBindings.DT_CombatVFXBindings"
PRODUCTION_NIAGARA = "/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash"
PRODUCTION_PIERCE_NIAGARA = "/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash.NS_Hero1AxePierce_MeshSlash"
PRODUCTION_PIERCE_BLADE_MESH = "/Game/VFX/Hero1/Axe/Pierce/SM_Hero1AxePierce_BladePlane.SM_Hero1AxePierce_BladePlane"
PRODUCTION_BOUNCE_NIAGARA = "/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash"
PRODUCTION_BOUNCE_SLASH_MESH = "/Game/VFX/Hero1/Axe/Bounce/SM_Hero1AxeBounce_HorizontalSlash.SM_Hero1AxeBounce_HorizontalSlash"
PRODUCTION_DOT_NIAGARA = "/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash.NS_Hero1AxeDOT_MeshSlash"
PRODUCTION_DOT_RING_MESH = "/Game/VFX/Hero1/Axe/DOT/SM_Hero1AxeDOT_AuraRing.SM_Hero1AxeDOT_AuraRing"
PRODUCTION_PREFIX = "/Game/VFX/Hero1/Axe"
LAB_PREFIX = "/Game/VFXLab"

ACTIVE_REQUIRED_FIELDS = (
    "BindingID",
    "SourceType",
    "SourceID",
    "AttackCategory",
    "NiagaraSystem",
    "EffectPacketID",
    "VFXProfile",
    "BaseVisualRadius",
    "BasePlaybackSeconds",
)

REQUIRED_ASSETS = [
    DT_ASSET,
    PRODUCTION_NIAGARA,
    "/Game/VFX/Hero1/Axe/Shared/SM_Hero1AxeAOE_SlashArc.SM_Hero1AxeAOE_SlashArc",
    "/Game/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright",
    "/Game/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body",
    "/Game/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark",
    "/Game/VFX/Hero1/Axe/Shared/T_Hero1AxeAOE_StreakMask.T_Hero1AxeAOE_StreakMask",
    "/Game/VFX/Hero1/Axe/Shared/T_Hero1AxeAOE_DissolveNoise.T_Hero1AxeAOE_DissolveNoise",
    "/Game/VFX/Hero1/Axe/Shared/T_Hero1AxeAOE_ImpactMask.T_Hero1AxeAOE_ImpactMask",
    PRODUCTION_PIERCE_NIAGARA,
    PRODUCTION_PIERCE_BLADE_MESH,
    PRODUCTION_BOUNCE_NIAGARA,
    PRODUCTION_BOUNCE_SLASH_MESH,
    PRODUCTION_DOT_NIAGARA,
    PRODUCTION_DOT_RING_MESH,
]

unreal = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate Combat VFX production bindings.")
    parser.add_argument("--self-test-root", default="")
    args, _unknown = parser.parse_known_args()
    return args


def require_unreal():
    global unreal
    if unreal is None:
        import unreal as unreal_module  # type: ignore

        unreal = unreal_module
    return unreal


def log(message):
    print(f"{LOG} {message}")
    if unreal is not None:
        unreal.log(f"{LOG} {message}")


def fail(message):
    print(f"{LOG} {message}", file=sys.stderr)
    if unreal is not None:
        unreal.log_error(f"{LOG} {message}")
    raise RuntimeError(message)


def read_binding_rows(csv_path: Path) -> list[dict[str, str]]:
    if not csv_path.exists():
        fail(f"Missing CSV: {csv_path}")
    with csv_path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle))


def binding_row_id(row: dict[str, str]) -> str:
    return row.get("BindingID") or row.get("---") or "<unknown>"


def is_deferred_scaffold_row(row: dict[str, str]) -> bool:
    notes = (row.get("Notes") or "").lower()
    niagara = (row.get("NiagaraSystem") or "").strip()
    return (
        not niagara
        or notes.startswith("deferred ")
        or "deferred scaffold" in notes
        or "scaffold-only" in notes
    )


def validate_active_binding_rows(rows: list[dict[str, str]], *, emit_failure: bool = True) -> dict[str, object]:
    active: list[str] = []
    deferred: list[str] = []
    failures: list[str] = []

    for row in rows:
        row_id = binding_row_id(row)
        if is_deferred_scaffold_row(row):
            deferred.append(row_id)
            continue

        active.append(row_id)
        for field in ACTIVE_REQUIRED_FIELDS:
            if not (row.get(field) or "").strip():
                failures.append(f"{row_id} missing {field}")

        niagara = (row.get("NiagaraSystem") or "").strip()
        if niagara.startswith(LAB_PREFIX):
            failures.append(f"{row_id} uses lab Niagara path {niagara}")

        for numeric_field in ("BaseVisualRadius", "BasePlaybackSeconds", "VisualScaleMultiplier"):
            raw_value = (row.get(numeric_field) or "").strip()
            if not raw_value:
                continue
            try:
                float(raw_value)
            except ValueError:
                failures.append(f"{row_id} {numeric_field} is not numeric: {raw_value!r}")

    if failures:
        message = "Active binding row validation failed: " + "; ".join(failures)
        if emit_failure:
            fail(message)
        raise RuntimeError(message)

    summary = {"active": active, "deferred": deferred, "active_count": len(active), "deferred_count": len(deferred)}
    log(f"Active binding rows: {active}; deferred scaffold rows: {deferred}")
    return summary


def validate_csv_binding():
    rows = read_binding_rows(CSV_PATH)
    validate_active_binding_rows(rows)

    row = next((candidate for candidate in rows if candidate.get("---") == "Hero1Axe_AOE_Base"), None)
    if row is None:
        fail("CombatVFXBindings.csv is missing Hero1Axe_AOE_Base row")

    expected = {
        "BindingID": "Hero1Axe_AOE_Base",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_1_black_aoe",
        "AttackCategory": "AOE",
        "NiagaraSystem": PRODUCTION_NIAGARA,
        "bSuppressTemporaryProjectile": "True",
    }
    for key, value in expected.items():
        if row.get(key) != value:
            fail(f"Hero1Axe_AOE_Base {key}={row.get(key)!r}, expected {value!r}")

    base_visual_radius = float(row.get("BaseVisualRadius") or 0.0)
    if abs(base_visual_radius - 411.4) > 0.05:
        fail(f"Hero1Axe_AOE_Base BaseVisualRadius={base_visual_radius:.3f}, expected 411.4")

    log("CSV binding row is production-bound, suppresses the temporary projectile, and uses the calibrated visual radius")


def validate_hero3_aoe_placeholder_csv_binding():
    rows = read_binding_rows(CSV_PATH)

    row = next((candidate for candidate in rows if candidate.get("---") == "Hero3_AOE_Black_Placeholder"), None)
    if row is None:
        fail("CombatVFXBindings.csv is missing Hero3_AOE_Black_Placeholder row")

    expected = {
        "BindingID": "Hero3_AOE_Black_Placeholder",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_3_black_aoe",
        "AttackCategory": "AOE",
        "NiagaraSystem": PRODUCTION_NIAGARA,
        "bSuppressTemporaryProjectile": "True",
    }
    for key, value in expected.items():
        if row.get(key) != value:
            fail(f"Hero3_AOE_Black_Placeholder {key}={row.get(key)!r}, expected {value!r}")

    notes = row.get("Notes") or ""
    if "FLAGGED placeholder reuse" not in notes:
        fail("Hero3_AOE_Black_Placeholder Notes must flag the temporary Hero 1 AOE reuse")

    log("Hero 3 AOE placeholder row reuses the Hero 1 AOE Niagara path and is explicitly flagged")


def validate_pierce_csv_binding():
    rows = read_binding_rows(CSV_PATH)

    row = next((candidate for candidate in rows if candidate.get("---") == "Hero2_Pierce_Black_Base"), None)
    if row is None:
        fail("CombatVFXBindings.csv is missing Hero2_Pierce_Black_Base row")

    expected = {
        "BindingID": "Hero2_Pierce_Black_Base",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_2_black_pierce",
        "AttackCategory": "Pierce",
        "NiagaraSystem": PRODUCTION_PIERCE_NIAGARA,
        "bSuppressTemporaryProjectile": "True",
    }
    for key, value in expected.items():
        if row.get(key) != value:
            fail(f"Hero2_Pierce_Black_Base {key}={row.get(key)!r}, expected {value!r}")

    log("Hero 2 Pierce CSV binding row is production-bound to the PathAnchored Niagara system and suppresses the temporary projectile")


def validate_bounce_csv_binding():
    rows = read_binding_rows(CSV_PATH)

    row = next((candidate for candidate in rows if candidate.get("---") == "Hero4_Bounce_Black_Base"), None)
    if row is None:
        fail("CombatVFXBindings.csv is missing Hero4_Bounce_Black_Base row")

    expected = {
        "BindingID": "Hero4_Bounce_Black_Base",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_4_black_bounce",
        "AttackCategory": "Bounce",
        "NiagaraSystem": PRODUCTION_BOUNCE_NIAGARA,
        "EffectPacketID": "Hero1AxeBounceMechanismPacket",
        "VFXProfile": "MeshSlashBounce",
        "bSuppressTemporaryProjectile": "True",
    }
    for key, value in expected.items():
        if row.get(key) != value:
            fail(f"Hero4_Bounce_Black_Base {key}={row.get(key)!r}, expected {value!r}")

    base_playback = float(row.get("BasePlaybackSeconds") or 0.0)
    if abs(base_playback - 0.32) > 0.005:
        fail(f"Hero4_Bounce_Black_Base BasePlaybackSeconds={base_playback:.3f}, expected 0.32")

    log("Hero 4 Bounce CSV binding row is production-bound to the ImpactAnchored per-link Niagara system and suppresses the temporary projectile")


def validate_dot_csv_binding():
    rows = read_binding_rows(CSV_PATH)

    row = next((candidate for candidate in rows if candidate.get("---") == "Hero5_DOT_Black_Base"), None)
    if row is None:
        fail("CombatVFXBindings.csv is missing Hero5_DOT_Black_Base row")

    if is_deferred_scaffold_row(row):
        fail("Hero5_DOT_Black_Base is still a deferred scaffold row; it must be an active production row")

    expected = {
        "BindingID": "Hero5_DOT_Black_Base",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_5_black_dot",
        "AttackCategory": "DOT",
        "NiagaraSystem": PRODUCTION_DOT_NIAGARA,
        "EffectPacketID": "Hero1AxeDOTMechanismPacket",
        "VFXProfile": "MeshSlashDOT",
        "bSuppressTemporaryProjectile": "True",
    }
    for key, value in expected.items():
        if row.get(key) != value:
            fail(f"Hero5_DOT_Black_Base {key}={row.get(key)!r}, expected {value!r}")

    log("Hero 5 DOT CSV binding row is production-bound to the moving aura-ring carrier Niagara system and suppresses the temporary projectile")


def validate_weapon_geometry_contract():
    if not WEAPONS_CSV_PATH.exists():
        fail(f"Missing weapons CSV: {WEAPONS_CSV_PATH}")

    with WEAPONS_CSV_PATH.open("r", encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.DictReader(handle))

    if not rows:
        fail("Weapons.csv has no rows")

    missing_field_rows = [row.get("---") or row.get("WeaponID") or "<unknown>" for row in rows if "AoeInnerRadiusRatio" not in row or row.get("AoeInnerRadiusRatio") in (None, "")]
    if missing_field_rows:
        fail(f"Weapons.csv rows missing AoeInnerRadiusRatio: {missing_field_rows[:12]!r}")

    expected_hero1_aoe_rows = {
        "Hero_1_black_aoe": ("Hero1CrescentSingle", 1),
        "Hero_1_red_aoe": ("Hero1CrescentTriple", 3),
        "Hero_1_yellow_aoe": ("Hero1CrescentFive", 5),
        "Hero_1_white_aoe": ("Hero1CrescentFullContact", 1),
    }
    rows_by_name = {row.get("---") or row.get("WeaponID") or "<unknown>": row for row in rows}
    for row_name, (expected_pattern, expected_projectiles) in expected_hero1_aoe_rows.items():
        hero_row = rows_by_name.get(row_name)
        if hero_row is None:
            fail(f"Weapons.csv is missing {row_name} row")

        hero_ratio = float(hero_row.get("AoeInnerRadiusRatio") or 0.0)
        if abs(hero_ratio - 0.54) > 0.001:
            fail(f"{row_name} AoeInnerRadiusRatio={hero_ratio:.3f}, expected 0.54")
        if hero_row.get("AttackPatternID") != expected_pattern:
            fail(f"{row_name} AttackPatternID={hero_row.get('AttackPatternID')!r}, expected {expected_pattern!r}")
        projectile_count = int(float(hero_row.get("ProjectileCount") or 0))
        if projectile_count != expected_projectiles:
            fail(f"{row_name} ProjectileCount={projectile_count}, expected {expected_projectiles}")

    bad_other_aoes = []
    for row in rows:
        row_name = row.get("---") or row.get("WeaponID") or "<unknown>"
        if row_name in expected_hero1_aoe_rows:
            continue
        if row.get("Branch") != "AOE":
            continue
        ratio = float(row.get("AoeInnerRadiusRatio") or 0.0)
        if abs(ratio) > 0.001:
            bad_other_aoes.append((row_name, ratio))
    if bad_other_aoes:
        fail(f"Only Hero 1 approved AOE placeholder rows may use the crescent-band inner ratio; unexpected AOE ratios: {bad_other_aoes[:12]!r}")

    log("Weapon geometry contract is present: Hero 1 AOE placeholder rows use crescent-band patterns and other AOE rows remain filled-sector")


def validate_required_assets():
    require_unreal()
    for asset_path in REQUIRED_ASSETS:
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            fail(f"Missing required production binding asset: {asset_path}")
    log("Required production DataTable/Niagara/material/mesh/texture assets exist")


def validate_game_instance_binding():
    require_unreal()
    bp_class = unreal.load_object(None, BP_GAME_INSTANCE_CLASS_PATH)
    if not bp_class:
        fail(f"Could not load BP_T66GameInstance class at {BP_GAME_INSTANCE_CLASS_PATH}")

    cdo = unreal.get_default_object(bp_class)
    if not cdo:
        fail("Could not resolve BP_T66GameInstance CDO")

    bound_table = cdo.get_editor_property("CombatVFXBindingsDataTable")
    if not bound_table:
        fail("BP_T66GameInstance CombatVFXBindingsDataTable is unset")

    if bound_table.get_path_name() != DT_ASSET:
        fail(f"BP_T66GameInstance CombatVFXBindingsDataTable={bound_table.get_path_name()}, expected {DT_ASSET}")

    log("BP_T66GameInstance points at DT_CombatVFXBindings")


def _asset_package_path(asset_data):
    try:
        return str(asset_data.package_name)
    except Exception:
        return str(asset_data.get_editor_property("package_name"))


def _dependency_paths(registry, package_path):
    require_unreal()
    options = unreal.AssetRegistryDependencyOptions(
        include_soft_package_references=True,
        include_hard_package_references=True,
        include_searchable_names=False,
        include_soft_management_references=False,
        include_hard_management_references=False,
    )
    return [str(dep) for dep in (registry.get_dependencies(unreal.Name(package_path), options) or [])]


def validate_no_production_dependency_on_lab():
    require_unreal()
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    failures = []
    for asset_data in registry.get_assets_by_path(unreal.Name(PRODUCTION_PREFIX), recursive=True):
        package_path = _asset_package_path(asset_data)
        for dep in _dependency_paths(registry, package_path):
            if dep.startswith(LAB_PREFIX):
                failures.append((package_path, dep))

    if failures:
        fail(f"Production VFX asset depends on lab asset: {failures[:12]!r}")
    log("Production VFX assets do not depend on /Game/VFXLab")


def _require_fragments(path, fragments, label):
    text = path.read_text(encoding="utf-8")
    missing = [fragment for fragment in fragments if fragment not in text]
    if missing:
        fail(f"{label} is missing required fragments: {missing!r}")


def validate_source_guards():
    _require_fragments(
        DATA_TYPES_H,
        [
            "AoeInnerRadiusRatio",
            "filled-sector",
        ],
        "Data types",
    )
    _require_fragments(
        COMBAT_COMPONENT_CPP,
        [
            "ResolveCombatVFXBinding",
            "ShouldSuppressWeaponBaseProjectileVisual",
            "TrySpawnBoundWeaponBaseSlashVFX",
            "CombatVFXProductionSpawned",
            "EffectiveSlashInnerRadius",
            "AoeInnerRadiusRatio",
            "GetCategorySubDamageMultiplier",
            "GetCategorySubAttackSpeedMultiplier",
            "GetCategorySubScaleMultiplier",
            "bPathAnchoredCarrier",
            "PathAnchored",
        ],
        "Combat component",
    )
    _require_fragments(
        COMBAT_SHARED_CPP,
        [
            "GetCategorySubDamageMultiplier",
            "GetCategorySubAttackSpeedMultiplier",
            "GetCategorySubScaleMultiplier",
        ],
        "Combat shared helpers",
    )
    _require_fragments(
        OVERLAYS_CPP,
        [
            "hero1axeaoevfxbinding",
            "Hero1AxeAOEVFXBindingProof",
            "ManualLabVFX=0",
            "InsideBandForward",
            "InsideAngleEdge",
            "InnerHollow",
            "OutsideAngleEdge",
            "OutsideRadius",
            "hero1axebouncevfxbinding",
            "ET66AttackCategory::Bounce",
        ],
        "Gameplay capture automation",
    )
    _require_fragments(
        CAPTURE_SCRIPT,
        [
            "hero1axeaoevfxbinding",
            "Hero1AxeProofItems",
            "T66Hero1AxeAOEProofItems",
            "hero1axebouncevfxbinding",
        ],
        "Gameplay video capture script",
    )
    log("Source guards for binding dispatch, item scaling, and production proof mode are present")


def run_unreal_validation() -> None:
    require_unreal()
    log("=== Combat VFX production binding validation ===")
    log(DISCLAIMER)
    validate_csv_binding()
    validate_hero3_aoe_placeholder_csv_binding()
    validate_pierce_csv_binding()
    validate_bounce_csv_binding()
    validate_dot_csv_binding()
    validate_weapon_geometry_contract()
    validate_required_assets()
    validate_game_instance_binding()
    validate_no_production_dependency_on_lab()
    validate_source_guards()
    log("=== Combat VFX production binding validation DONE ===")


def write_self_test_csv(path: Path, rows: list[dict[str, str]]) -> None:
    fieldnames = ["---", *ACTIVE_REQUIRED_FIELDS, "bSuppressTemporaryProjectile", "bDevelopmentFallbackAllowed", "VisualScaleMultiplier", "Notes"]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def run_self_test(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    positive_csv = root / "positive_bindings.csv"
    negative_csv = root / "negative_bindings.csv"
    report_path = root / "self_test_report.json"

    active_row = {
        "---": "Hero1Axe_AOE_Base",
        "BindingID": "Hero1Axe_AOE_Base",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_1_black_aoe",
        "AttackCategory": "AOE",
        "NiagaraSystem": PRODUCTION_NIAGARA,
        "EffectPacketID": "Hero1AxeAOESlashMechanismPacket",
        "VFXProfile": "MeshSlashAOE",
        "BaseVisualRadius": "411.4",
        "BasePlaybackSeconds": "0.46",
        "VisualScaleMultiplier": "1.0",
        "bSuppressTemporaryProjectile": "True",
        "bDevelopmentFallbackAllowed": "True",
        "Notes": "active synthetic row",
    }
    deferred_row = {
        "---": "Hero5_DOT_Scaffold",
        "BindingID": "Hero5_DOT_Scaffold",
        "SourceType": "WeaponBase",
        "SourceID": "Hero_5_black_dot",
        "AttackCategory": "DOT",
        "NiagaraSystem": "",
        "EffectPacketID": "Hero1AxeDOTMechanismPacket",
        "VFXProfile": "",
        "BaseVisualRadius": "",
        "BasePlaybackSeconds": "",
        "VisualScaleMultiplier": "",
        "bSuppressTemporaryProjectile": "",
        "bDevelopmentFallbackAllowed": "False",
        "Notes": "DEFERRED scaffold-only row",
    }
    malformed_row = dict(active_row)
    malformed_row["BindingID"] = "Malformed_Active"
    malformed_row["NiagaraSystem"] = "/Game/VFX/Hero1/Axe/AOE/NS_Test.NS_Test"
    malformed_row["EffectPacketID"] = ""

    write_self_test_csv(positive_csv, [active_row, deferred_row])
    write_self_test_csv(negative_csv, [malformed_row])

    positive_summary = validate_active_binding_rows(read_binding_rows(positive_csv))
    if positive_summary["active_count"] != 1 or positive_summary["deferred_count"] != 1:
        fail(f"Unexpected self-test positive summary: {positive_summary!r}")

    negative_failed = False
    try:
        validate_active_binding_rows(read_binding_rows(negative_csv), emit_failure=False)
    except RuntimeError:
        negative_failed = True
    if not negative_failed:
        fail("Negative malformed active row did not fail")

    report = {
        "schema": "t66.combat_vfx_binding_validator.self_test.v1",
        "disclaimer": DISCLAIMER,
        "positive_csv": str(positive_csv),
        "negative_csv": str(negative_csv),
        "positive_summary": positive_summary,
        "negative_failed": negative_failed,
    }
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"{LOG} {DISCLAIMER}")
    print(f"{LOG} SELF TEST PASSED")
    print(f"{LOG} Self-test report: {report_path}")


def main() -> int:
    args = parse_args()
    try:
        if args.self_test_root:
            run_self_test(Path(args.self_test_root))
        else:
            run_unreal_validation()
    except Exception as exc:
        if unreal is not None:
            unreal.log_error(f"{LOG} Validation failed: {exc}")
        else:
            print(f"{LOG} Validation failed: {exc}", file=sys.stderr)
        return 1
    return 0


sys.exit(main())
