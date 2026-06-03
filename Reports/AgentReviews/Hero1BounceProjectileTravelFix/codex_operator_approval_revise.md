Codex Approval: APPROVE

# Codex Operator Approval - Hero 1 Bounce Projectile Travel Fix Revision

Approved operator: Claude
Validator/integrator: Codex
Tool profile: FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`

## Approved Task

Revise the first Bounce projectile travel fix so the moving primary carrier is the authored Hero 1 Bounce red/blue horizontal slash VFX, not the generic temporary Bounce projectile profile. Preserve the sequential hero->primary then primary->second timing and existing Bounce damage/context authority.

## Approved Scope

Allowed edits:

- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66HeroProjectile.h`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` only to preserve/strengthen the two-target proof harness
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- report files under `Reports/AgentReviews/Hero1BounceProjectileTravelFix/`
- `Content/Data/CombatVFXBindings.csv` and `Scripts/ValidateCombatVFXProductionBindings.py` only if strictly required by the revised carrier contract

## Approved Tool Surface

Full Claude Operator tool surface through the local Claude Code CLI, using the user's Claude subscription. Shell/build/editor commands are approved only inside the task scope.

## Required Process Rules

- Preserve combat damage authority and per-link impact contexts.
- The visible primary moving carrier must be the authored Bounce slash VFX or an equivalent Niagara/material/renderer carrier.
- Temporary projectile mesh/profile may be used only as hidden mover/support, not as the accepted primary silhouette.
- Do not reintroduce static impact-only slashes or simultaneous multi-projectile proof.
- Keep Mini/minigame scope excluded.

## Explicitly Excluded Actions

- Do not touch unrelated attack families, idols, balance/stat tuning, Mini/minigame code, Git operations, broad Git/LFS scans, or unrelated working-tree changes.

## Verification Required After Operator Run

- Focused `T66Editor Win64 Development` compile.
- Validator if binding/source-guard assumptions change.
- Report with carrier method, visibility timing, commands, and caveats for Codex capture.

## Approval Rationale

Codex validation found the first Operator artifact was only partial: it fixed sequencing but used a temporary projectile profile as the primary visual carrier. The revision is narrow and required to preserve the Combat VFX method class and the user's horizontal-slash projectile intent.
