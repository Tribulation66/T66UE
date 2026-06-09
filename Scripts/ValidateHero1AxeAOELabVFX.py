"""
Validate the isolated Hero 1 axe AOE Niagara lab assets and cook isolation.

Run with:
  "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
    "C:/UE/T66/T66.uproject" -run=pythonscript -script="C:/UE/T66/Scripts/ValidateHero1AxeAOELabVFX.py" -unattended -nop4 -nosplash
"""

import configparser
import re
from pathlib import Path

import unreal


LOG = "[Hero1AxeAOELabVFXValidate]"
PROJECT_ROOT = Path(r"C:/UE/T66")
DEFAULT_GAME_INI = PROJECT_ROOT / "Config" / "DefaultGame.ini"
LAB_ACTOR_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66Hero1AxeAOEVFXLabActor.cpp"
LAB_ACTOR_H = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66Hero1AxeAOEVFXLabActor.h"
COMMANDLET_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66Hero1AxeAOEVFXCommandlet.cpp"
OVERLAYS_CPP = PROJECT_ROOT / "Source" / "T66" / "Gameplay" / "T66PlayerController_Overlays.cpp"
CAPTURE_SCRIPT = PROJECT_ROOT / "Scripts" / "CaptureT66GameplayVideo.ps1"
SETUP_SCRIPT = PROJECT_ROOT / "Scripts" / "SetupHero1AxeAOELabVFX.py"
LAB_PREFIX = "/Game/VFXLab"
LAB_AOE_PREFIX = "/Game/VFXLab/Hero1Axe/AOE"
LAB_SHARED_PREFIX = "/Game/VFXLab/Hero1Axe/Shared"

REQUIRED_ASSETS = [
    f"{LAB_AOE_PREFIX}/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_ImpactFlare.M_Hero1AxeAOE_ImpactFlare",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_DirectionalSpark.M_Hero1AxeAOE_DirectionalSpark",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Mote.M_Hero1AxeAOE_Mote",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_GroundTrace.M_Hero1AxeAOE_GroundTrace",
    f"{LAB_SHARED_PREFIX}/SM_Hero1AxeAOE_SlashArc.SM_Hero1AxeAOE_SlashArc",
    f"{LAB_SHARED_PREFIX}/T_Hero1AxeAOE_StreakMask.T_Hero1AxeAOE_StreakMask",
    f"{LAB_SHARED_PREFIX}/T_Hero1AxeAOE_DissolveNoise.T_Hero1AxeAOE_DissolveNoise",
    f"{LAB_SHARED_PREFIX}/T_Hero1AxeAOE_ImpactMask.T_Hero1AxeAOE_ImpactMask",
]

DEPRECATED_ASSETS = [
    f"{LAB_AOE_PREFIX}/NS_Hero1AxeAOE_CrescentSlash.NS_Hero1AxeAOE_CrescentSlash",
    f"{LAB_AOE_PREFIX}/P_Hero1AxeAOE_WeaponSlashSeed.P_Hero1AxeAOE_WeaponSlashSeed",
    f"{LAB_AOE_PREFIX}/P_Hero1AxeAOE_ShockwaveSeed.P_Hero1AxeAOE_ShockwaveSeed",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Reveal.M_Hero1AxeAOE_Slash_Reveal",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_CrescentSlash.M_Hero1AxeAOE_CrescentSlash",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_VertexOpaque.M_Hero1AxeAOE_VertexOpaque",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_VertexAdditive.M_Hero1AxeAOE_VertexAdditive",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_VertexTranslucent.M_Hero1AxeAOE_VertexTranslucent",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_AxeConstant.M_Hero1AxeAOE_AxeConstant",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_WeaponArt.M_Hero1AxeAOE_WeaponArt",
    f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_WeaponArtGlow.M_Hero1AxeAOE_WeaponArtGlow",
]

ALLOWED_LAB_DEPENDENCY_PREFIXES = (
    "/Game/VFXLab",
    "/Script/",
    "/Niagara/",
    "/Engine/",
)


def log(message):
    unreal.log(f"{LOG} {message}")


def fail(message):
    unreal.log_error(f"{LOG} {message}")
    raise RuntimeError(message)


def _extract_paths(lines, key):
    result = []
    pattern = re.compile(rf"^\s*\+?{re.escape(key)}=\(Path=\"([^\"]+)\"\)")
    for line in lines:
        match = pattern.search(line)
        if match:
            result.append(match.group(1))
    return result


def validate_config():
    parser = configparser.ConfigParser(strict=False)
    parser.optionxform = str
    text = DEFAULT_GAME_INI.read_text(encoding="utf-8")
    if "[/Script/UnrealEd.ProjectPackagingSettings]" not in text:
        fail("DefaultGame.ini is missing ProjectPackagingSettings section")

    lines = text.splitlines()
    never_cook = _extract_paths(lines, "DirectoriesToNeverCook")
    always_cook = _extract_paths(lines, "DirectoriesToAlwaysCook")

    if LAB_PREFIX not in never_cook:
        fail(f"{LAB_PREFIX} is not in DirectoriesToNeverCook")
    if LAB_PREFIX in always_cook:
        fail(f"{LAB_PREFIX} is unexpectedly in DirectoriesToAlwaysCook")

    log(f"Cook config OK: never-cook contains {LAB_PREFIX}; always-cook does not")


def validate_assets_exist():
    for asset_path in REQUIRED_ASSETS:
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            fail(f"Missing required mesh-slash lab asset: {asset_path}")
    for asset_path in DEPRECATED_ASSETS:
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            fail(f"Deprecated sprite/procedural lab asset still exists: {asset_path}")
    log("Required mesh slash assets exist and deprecated sprite/procedural assets are absent")


def validate_layer_material_assets():
    expected_blends = {
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright": unreal.BlendMode.BLEND_MASKED,
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body": unreal.BlendMode.BLEND_MASKED,
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark": unreal.BlendMode.BLEND_MASKED,
    }
    for asset_path, expected_blend in expected_blends.items():
        material = unreal.EditorAssetLibrary.load_asset(asset_path)
        if material is None:
            fail(f"Missing required slash layer material: {asset_path}")
        actual_blend = material.get_editor_property("blend_mode")
        if actual_blend != expected_blend:
            fail(f"Slash layer material {asset_path} has blend {actual_blend}, expected {expected_blend}")
        used_with_niagara = bool(material.get_editor_property("used_with_niagara_mesh_particles"))
        if not used_with_niagara:
            fail(f"Slash layer material {asset_path} is not marked used_with_niagara_mesh_particles")
    log("Slash layer material assets have required additive/translucent blend modes and Niagara mesh usage")


def validate_support_material_assets():
    expected_blends = {
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_ImpactFlare.M_Hero1AxeAOE_ImpactFlare": unreal.BlendMode.BLEND_ADDITIVE,
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_DirectionalSpark.M_Hero1AxeAOE_DirectionalSpark": unreal.BlendMode.BLEND_ADDITIVE,
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_Mote.M_Hero1AxeAOE_Mote": unreal.BlendMode.BLEND_ADDITIVE,
        f"{LAB_SHARED_PREFIX}/M_Hero1AxeAOE_GroundTrace.M_Hero1AxeAOE_GroundTrace": unreal.BlendMode.BLEND_TRANSLUCENT,
    }
    for asset_path, expected_blend in expected_blends.items():
        material = unreal.EditorAssetLibrary.load_asset(asset_path)
        if material is None:
            fail(f"Missing required support material: {asset_path}")
        actual_blend = material.get_editor_property("blend_mode")
        if actual_blend != expected_blend:
            fail(f"Support material {asset_path} has blend {actual_blend}, expected {expected_blend}")
        used_with_niagara = bool(material.get_editor_property("used_with_niagara_sprites"))
        if not used_with_niagara:
            fail(f"Support material {asset_path} is not marked used_with_niagara_sprites")
    log("Support materials exist with required blend modes and Niagara sprite usage")


def _asset_path_from_data(asset_data):
    try:
        return str(asset_data.package_name)
    except Exception:
        return str(asset_data.get_editor_property("package_name"))


def _dependency_paths(registry, package_path):
    package_name = unreal.Name(package_path)
    options = unreal.AssetRegistryDependencyOptions(
        include_soft_package_references=True,
        include_hard_package_references=True,
        include_searchable_names=False,
        include_soft_management_references=False,
        include_hard_management_references=False,
    )
    deps = registry.get_dependencies(package_name, options) or []
    return [str(dep) for dep in deps]


def _is_lab_asset(path):
    return path.startswith(LAB_PREFIX)


def _is_allowed_lab_dependency(path):
    return any(path.startswith(prefix) for prefix in ALLOWED_LAB_DEPENDENCY_PREFIXES)


def validate_asset_registry():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    all_assets = registry.get_all_assets()

    live_to_lab = []
    lab_to_live = []

    for asset_data in all_assets:
        package_path = _asset_path_from_data(asset_data)
        deps = _dependency_paths(registry, package_path)

        if _is_lab_asset(package_path):
            for dep in deps:
                if not _is_allowed_lab_dependency(dep):
                    lab_to_live.append((package_path, dep))
        else:
            for dep in deps:
                if _is_lab_asset(dep):
                    live_to_lab.append((package_path, dep))

    if live_to_lab:
        fail("Live asset references /Game/VFXLab: " + repr(live_to_lab[:12]))
    if lab_to_live:
        fail("/Game/VFXLab asset references unapproved production asset: " + repr(lab_to_live[:12]))

    log("AssetRegistry isolation OK in both directions")


def _require_fragments(path, fragments, label):
    text = path.read_text(encoding="utf-8")
    missing = [fragment for fragment in fragments if fragment not in text]
    if missing:
        fail(f"{label} is missing required fragments: {missing!r}")
    return text


def _forbid_fragments(path, fragments, label):
    text = path.read_text(encoding="utf-8")
    present = [fragment for fragment in fragments if fragment in text]
    if present:
        fail(f"{label} still contains forbidden fragments: {present!r}")


def _require_order(path, first, second, label):
    text = path.read_text(encoding="utf-8")
    first_index = text.find(first)
    second_index = text.find(second)
    if first_index < 0 or second_index < 0 or first_index >= second_index:
        fail(f"{label} does not place {first!r} before {second!r}")


def validate_runtime_source_guards():
    _require_fragments(
        LAB_ACTOR_CPP,
        [
            "NS_Hero1AxeAOE_MeshSlash",
            "SlashNiagaraComponent",
            "RestartSlashIfNeeded",
            "DeactivateImmediate();",
            "Activate(true);",
            "SetCustomTimeDilation(1.0f)",
            "CycleDuration",
        ],
        "Lab actor source",
    )
    _require_fragments(
        LAB_ACTOR_H,
        [
            "SlashNiagaraComponent",
            "RestartSlashIfNeeded",
        ],
        "Lab actor header",
    )
    _require_fragments(
        SETUP_SCRIPT,
        [
            "NS_Hero1AxeAOE_MeshSlash",
            "SLASH_LAYER_CONFIGS",
            "M_Hero1AxeAOE_Slash_Bright",
            "M_Hero1AxeAOE_Slash_Body",
            "M_Hero1AxeAOE_Slash_Dark",
            "SUPPORT_MATERIAL_CONFIGS",
            "M_Hero1AxeAOE_ImpactFlare",
            "M_Hero1AxeAOE_DirectionalSpark",
            "M_Hero1AxeAOE_Mote",
            "M_Hero1AxeAOE_GroundTrace",
            "build_support_material",
            "\"SupportMask\"",
            "used_with_niagara_sprites",
            "MaterialExpressionDynamicParameter",
            "MaterialExpressionTextureSampleParameter2D",
            "\"SlashAge\"",
            "T_Hero1AxeAOE_StreakMask",
            "T_Hero1AxeAOE_DissolveNoise",
            "T_Hero1AxeAOE_ImpactMask",
            "\"StreakMask\"",
            "\"DissolveNoise\"",
            "\"ImpactMask\"",
            "DissolvePan",
            "DetailFloor",
            "GlowStrength",
            "OpacityBoost",
            "ImpactStrength",
            "ImpactColor",
            "BLEND_ADDITIVE",
            "BLEND_TRANSLUCENT",
            "MP_BASE_COLOR",
            "RevealLead",
            "RevealSharpness",
            "WidthSoftness",
            "\"Bright\"",
            "\"Body\"",
            "\"Dark\"",
            "radial_bias_mode",
            "\"inner\"",
            "\"outer\"",
            "RadialBiasMin",
            "radial_bias_power",
        ],
        "Setup script",
    )
    _require_fragments(
        COMMANDLET_CPP,
        [
            "SM_Hero1AxeAOE_SlashArc",
            "M_Hero1AxeAOE_Slash_Bright",
            "M_Hero1AxeAOE_Slash_Body",
            "M_Hero1AxeAOE_Slash_Dark",
            "M_Hero1AxeAOE_ImpactFlare",
            "M_Hero1AxeAOE_DirectionalSpark",
            "M_Hero1AxeAOE_Mote",
            "M_Hero1AxeAOE_GroundTrace",
            "FT66SlashLayerConfig",
            "FT66SupportEmitterConfig",
            "T66Hero1AxeAOESupportEmitterConfigs",
            "T66FindSupportEmitterConfig",
            "T66AddSupportEmitter",
            "T66Hero1AxeAOECarrierOnly",
            "CarrierOnly=%s",
            "FT66SlashProfileSample",
            "SlashProfileSamples",
            "0.00f, -90.0f",
            "1.00f, 90.0f",
            "Emitter_AxeAOESlash_Bright",
            "Emitter_AxeAOESlash_Body",
            "Emitter_AxeAOESlash_Dark",
            "Emitter_AxeAOEImpact_Flare",
            "Emitter_AxeAOESupport_DirectionalSparks",
            "Emitter_AxeAOESupport_Motes",
            "Emitter_AxeAOESupport_GroundTrace",
            "InitializeSystem(SlashSystem, true)",
            "UNiagaraMeshRendererProperties",
            "UNiagaraSpriteRendererProperties",
            "FNiagaraMeshRendererMeshProperties",
            "BuildFromMeshDescriptions",
            "bUsedWithNiagaraMeshParticles",
            "bUsedWithNiagaraSprites",
            "SourceMode = ENiagaraRendererSourceDataMode::Particles",
            "InitialMeshRotation.InitialMeshRotation",
            "DynamicMaterialParameters.DynamicMaterialParameters",
            "MeshRotationForce.MeshRotationForce",
            "SolveForcesAndVelocity.SolveForcesAndVelocity",
            "SolveRotationalForcesAndVelocity.SolveRotationalForcesAndVelocity",
            "MeshRotationForce-before-SolveRotationalForcesAndVelocity",
            "RotationForceZ",
            "Lever Radius (cm)",
            "T66LinkModuleInputToParticleParameter",
            "SYS_PARAM_PARTICLES_NORMALIZED_AGE",
            "SYS_PARAM_PARTICLES_POSITION",
            "SYS_PARAM_PARTICLES_VELOCITY",
            "SYS_PARAM_PARTICLES_SPRITE_SIZE",
            "SYS_PARAM_PARTICLES_SPRITE_FACING",
            "SYS_PARAM_PARTICLES_SPRITE_ALIGNMENT",
            "T66SetRapidIterationParameter<int32>",
            "T66SetRapidIterationParameter<FNiagaraBool>",
            "TEXT(\"Spawn Count\")",
            "TEXT(\"Spawn Probability\")",
            "TEXT(\"Pitch\")",
            "0.32f",
            "0.38f",
            "0.46f",
            "FRotator::ZeroRotator",
            "RemoveRenderer",
            "AddRenderer",
            "BoundSupportRendererCount",
            "RequestCompile(true)",
        ],
        "Niagara mesh commandlet",
    )
    _require_fragments(
        OVERLAYS_CPP,
        [
            "T66Hero1AxeAOESpawnTargets",
            "hero1axeaoehitbox",
            "PerformAutomationAutoAttackNow",
            "T66Hero1AxeAOEHitboxFireDelay",
            "T66Hero1AxeAOEHitboxVFXLeadSeconds",
            "AT66Hero1AxeAOEVFXLabActor::StaticClass()",
            "[Hero1AxeAOEHitboxProof] VFXSpawned",
            "[Hero1AxeAOEHitboxProof] Fire WorldTime",
            "[Hero1AxeAOEHitboxProof] DamageNumber",
            "UT66FloatingCombatTextSubsystem::EventType_Crit",
            "FloatingText->ShowDamageNumber(Target, DamageDelta",
            "DrawDebugString(",
            "GEngine->AddOnScreenDebugMessage(",
            "T66Automation_Hero1AxeAOETarget",
            "GameMode->SetEnemyDirectorSpawningPaused(true);",
            "cleared %d existing enemies for Hero1AxeAOE preview",
            "Enemy->ConfigureAsMob(TargetMobID);",
            "Movement->GravityScale = 0.0f;",
            "Movement->SetMovementMode(MOVE_None);",
            "Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT(\"HitReact\")), 30.0f);",
            "Enemy->SetActorTickEnabled(false);",
        ],
        "Gameplay capture automation",
    )
    _require_fragments(
        CAPTURE_SCRIPT,
        [
            "-T66Hero1AxeAOESpawnTargets",
            "Hero1AxeTargetCount",
            "Hero1AxeTargetForwardOffset",
            "Hero1AxeHitboxFireDelay",
            "Hero1AxeHitboxVFXLeadSeconds",
            "-T66Hero1AxeAOEHitboxFireDelay=$Hero1AxeHitboxFireDelay",
            "-T66Hero1AxeAOEHitboxVFXLeadSeconds=$Hero1AxeHitboxVFXLeadSeconds",
            "T66.Combat.DebugView 2",
            "NoHero1AxeTargets",
        ],
        "Gameplay video capture script",
    )

    overlays_text = OVERLAYS_CPP.read_text(encoding="utf-8")
    mode_index = overlays_text.find("hero1axeaoehitbox")
    guard_index = overlays_text.rfind("#if !UE_BUILD_SHIPPING", 0, mode_index)
    guard_end_index = overlays_text.find("#endif", mode_index)
    if mode_index < 0 or guard_index < 0 or guard_end_index < 0:
        fail("Hero1AxeAOE hitbox proof mode is not under a non-shipping guard")

    forbidden_fragments = [
        "UProceduralMeshComponent",
        "UStaticMeshComponent",
        "NS_Hero1AxeAOE_CrescentSlash",
        "M_Hero1AxeAOE_CrescentSlash",
        "MaterialExpressionSphereMask",
        "MaterialExpressionParticleColor",
        "SpawnRate.SpawnRate",
        "SlashBackingNiagaraComponent",
        "SlashCoreNiagaraComponent",
        "SlashHighlightNiagaraComponent",
        "SetVariableVec2",
        "SetVariableLinearColor",
        "T66NiagaraSpriteSizeParam",
        "T66NiagaraTintParam",
        "T66NiagaraColorParam",
        "BuildArcBand",
        "BuildSlashMeshes",
        "BuildAxeSilhouetteMesh",
        "BuildSparkMesh",
        "SlashArcNiagaraComponents",
        "WeaponSlashNiagaraComponent",
        "ShockwaveNiagaraComponent",
        "P_Hero1AxeAOE_WeaponSlashSeed",
        "P_Hero1AxeAOE_ShockwaveSeed",
        "P_Weapon_03",
        "P_Shockwave_Expl_03",
        "AxeArt",
        "LegacyProcedural",
        "T66Hero1AxeAOELayer",
        "Hero_1_black_aoe",
        "SetCustomTimeDilation(0.25f)",
    ]
    _forbid_fragments(LAB_ACTOR_CPP, forbidden_fragments, "Lab actor source")
    _forbid_fragments(LAB_ACTOR_H, forbidden_fragments, "Lab actor header")
    _forbid_fragments(COMMANDLET_CPP, forbidden_fragments, "Niagara mesh commandlet")
    _forbid_fragments(COMMANDLET_CPP, ["-104.0f", "104.0f"], "Niagara mesh commandlet arc span")
    _require_order(
        COMMANDLET_CPP,
        "MeshRotationForce.MeshRotationForce",
        "SolveRotationalForcesAndVelocity.SolveRotationalForcesAndVelocity",
        "Niagara mesh commandlet",
    )

    # The setup script is allowed to name deprecated assets only to delete them.
    setup_forbidden_fragments = [
        "UProceduralMeshComponent",
        "UStaticMeshComponent",
        "MaterialExpressionSphereMask",
        "MaterialExpressionParticleColor",
        "MaterialExpressionParticleRelativeTime",
        "SOURCE_WEAPON_NIAGARA",
        "SOURCE_SHOCKWAVE_NIAGARA",
        "P_Weapon_03",
        "P_Shockwave_Expl_03",
        "AxeArt",
        "LegacyProcedural",
        "Hero_1_black_aoe",
    ]
    _forbid_fragments(SETUP_SCRIPT, setup_forbidden_fragments, "Setup script")
    _forbid_fragments(CAPTURE_SCRIPT, ["Hero1AxeLayer", "T66Hero1AxeAOELayer"], "Gameplay video capture script")
    log("Runtime source guards OK: primary slash carrier is Niagara mesh/material-driven")


def validate_self_tests():
    synthetic_live = "/Game/VFX/NS_PixelParticle"
    synthetic_lab = "/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash"
    synthetic_stpack = "/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_03"
    synthetic_script = "/Script/Engine"
    synthetic_niagara = "/Niagara/Modules/Emitter/EmitterState"
    synthetic_engine = "/Engine/EngineMaterials/DefaultSpriteMaterial"

    if not _is_lab_asset(synthetic_lab):
        fail("Self-test failed: lab path was not detected as lab")
    if _is_lab_asset(synthetic_live):
        fail("Self-test failed: live path was detected as lab")
    if _is_allowed_lab_dependency(synthetic_live):
        fail("Self-test failed: production VFX dependency was allowed")
    if _is_allowed_lab_dependency(synthetic_stpack):
        fail("Self-test failed: generic StPack seed dependency was allowed")
    if not _is_allowed_lab_dependency(synthetic_script):
        fail("Self-test failed: script dependency was not allowed")
    if not _is_allowed_lab_dependency(synthetic_niagara):
        fail("Self-test failed: Niagara module dependency was not allowed")
    if not _is_allowed_lab_dependency(synthetic_engine):
        fail("Self-test failed: Engine material dependency was not allowed")

    log("Validator negative-path self-tests OK")


def main():
    validate_config()
    validate_assets_exist()
    validate_layer_material_assets()
    validate_support_material_assets()
    validate_self_tests()
    validate_asset_registry()
    validate_runtime_source_guards()
    log("DONE: structural validation passed; visual frame review is still required")


if __name__ == "__main__":
    main()
