# VFX Durable Baseline Proof Summary

Date: 2026-05-28

## Scope

This proof packet supports the durable Combat VFX infrastructure baseline. It does not claim final Hero 1 AOE visual-polish approval.

Synthetic self-test raw frames/contact sheets/video were not staged. The durable self-test evidence is the saved console output plus `EvidenceBundleSelfTest/self_test_report.json`; the removed raw synthetic media is regenerable by rerunning the self-test.

The parent proof run marker `.report-run.json` applies to this full baseline proof folder. The child evidence folders `EvidenceBundleSelfTest`, `BindingValidatorSelfTest`, and `AutoFrameSelection_Hero1AxeAOE_EdgeFinal` also carry their own `.report-run.json` markers pointing back to this summary so future cleanup passes can apply the 15-day raw-run rule without guessing.

## Script Gates

- Evidence bundle self-test: `EvidenceBundleSelfTest_output.txt`
  - Result: `SELF TEST PASSED`.
  - Proves default fixed frame indices remain the default, manual frame labels still override, and opt-in auto activity selection records its method in the manifest.
- Binding validator self-test: `BindingValidatorSelfTest_output.txt`
  - Result: `SELF TEST PASSED`.
  - Proves active binding rows, deferred scaffold rows, and malformed active-row failure without importing Unreal.
- Weapon data structured diff: `WeaponsCsvAoeInnerRadiusDiff_summary.txt`
  - Result: only the `AoeInnerRadiusRatio` column was added; no rows were added or removed; no pre-existing column values changed; the only non-zero `AoeInnerRadiusRatio` value is `Hero_1_black_aoe = 0.54`.
- Scripts pending issues diff: `ScriptsPendingIssuesDiff_snippet.txt`
  - Result: staged hunk appends exactly two VFX-resolved entries; existing non-VFX pending issues remain unchanged.
- Unreal binding validator: `ValidateCombatVFXProductionBindings_Unreal.log`
  - Result: completion token `=== Combat VFX production binding validation DONE ===`.
  - Error scan: no `LogPython: Error`, `LogOutputDevice: Error`, or `Traceback`.
- Hero 1 AOE binding proof wrapper print-only diff:
  - Pre-change: `RunHero1AxeAOEVFXBindingProof_printonly_prechange.txt`
  - Post-change: `RunHero1AxeAOEVFXBindingProof_printonly_postchange.txt`
  - Opt-in auto-frame print-only smoke: `RunHero1AxeAOEVFXBindingProof_printonly_autoframe.txt`
  - Normalized diff: `RunHero1AxeAOEVFXBindingProof_printonly_normalized_diff.txt`
  - Result: normalized default output matches, so the opt-in auto-frame switch does not change normal production proof arguments.

## Auto Frame Evidence

- Retained frame source: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/frames`
- Auto evidence bundle: `AutoFrameSelection_Hero1AxeAOE_EdgeFinal/manifest.json`
- Selected indices: `start=51`, `mid=56`, `impact=62`, `dissipate=71`.
- Selection method: `auto_activity`.

## Existing Production Proof References

- Runtime binding/item-stat proof batch: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Hero1AxeAOEVFXBindingProofSummary.md`
- Baseline proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Baseline/proof_log_excerpt.md`
- AOE scale proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeScale/proof_log_excerpt.md`
- AOE speed proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeSpeed/proof_log_excerpt.md`
- AOE damage proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeDamage/proof_log_excerpt.md`
- Crescent-band hitbox cleanup video: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/Hero1AxeAOE_HitboxCleanup.mp4`
- Crescent-band hitbox cleanup contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/contact_sheet.png`
- Crescent-band hitbox cleanup runtime log: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log`

These existing production references are the normal-flow evidence for real weapon selection, RunState/inventory stat scaling, combat fire, VFX binding lookup, damage numbers, and crescent-band hitbox alignment. The new commandlet validator is an additional hard binding/asset gate; it is not the only production-path proof.

## Binding Row Scope

- `Content/Data/CombatVFXBindings.csv` contains exactly one active row for this baseline: `Hero1Axe_AOE_Base`, `SourceType=WeaponBase`, `SourceID=Hero_1_black_aoe`, `AttackCategory=AOE`, `NiagaraSystem=/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`.
- DOT, Pierce, Bounce, and idol overlay work are infrastructure/documentation only in this commit. There are no active runtime rows or non-empty Niagara paths for those future effects.

## Generated Runtime Asset Identity

- Pre-validation identity: `generated_runtime_assets_identity_prevalidation.json`
- Post-validation identity: `generated_runtime_assets_identity_postvalidation.json`
- Result: hashes, sizes, and mtimes are unchanged across validation.
- Postvalidation quoted identity: `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset` SHA256 `3025881EF90532FEAA3453B27C097AD2861C797A04E7E8CC6EB43A3911BBAAF4`, 1772344 bytes.
- Weapon hitbox geometry data is staged with the VFX baseline because `AoeInnerRadiusRatio` drives the Hero 1 AOE crescent-band hitbox contract used by the validator: `Content/Data/Weapons.csv` SHA256 `EB5AC1DBBA0FCAF91198A41C1378EF9A2953A73DB1304A6E35AEDABB9DB44408`, 60403 bytes, and `Content/Data/DT_Weapons.uasset` SHA256 `820B6439833D7E71B21A5588B92CE2D79270ECE265680280D97A1B572FBABB4D`, 272408 bytes.
