You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\TNTInteractable\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\TNTInteractable\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\TNTInteractable\20260607T160516-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Prompt

I want to build something that isnt a obstacle or trap but rather its an interactable called TNT and what it does is after you interact with and a few seconds go by it explodes, and damages everything around it, heros and enemies.

# Task Contract

Working task:
Operator: Codex
Validator: Claude, if the local validator path is available
Scope: add a new TNT world interactable that can be triggered by player interaction, waits a few seconds, then explodes and damages nearby heroes and enemies; integrate it with existing gameplay/interactable patterns and verify the build/runtime path.
Stop condition: implementation is complete, focused verification is run, and any unverified runtime behavior is clearly called out.

# Repo Rules

- Root process router: `C:\UE\T66\AGENTS.md`.
- Operator/Validator protocol: `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`.
- Current role state: Codex operator, Claude validator.
- Gameplay router: `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`.
- World router: `C:\UE\T66\Gameplay\World\WORLD_AGENTS.md`.
- Existing interactable base: `Source/T66/Gameplay/T66WorldInteractableBase.*`.
- Prefer data-authored tuning where practical, but a narrow first-pass C++ default is acceptable if scoped and editable.
- Runtime-facing gameplay changes require focused compile and staged standalone validation.

# Current Assumptions For This Pass

- TNT is not a trap or obstacle and should not be added to the trap subsystem.
- TNT should be a reusable `AT66WorldInteractableBase` subclass.
- TNT should be Lab Collector-spawnable for live testing.
- TNT should not yet be randomly spawned into tower generation unless the user asks for map distribution/balance.
- Placeholder primitive visuals are acceptable for this gameplay pass; no new Pixal3D asset generation/import is in scope.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

Build TNT as a new `AT66TNTInteractable : public AT66WorldInteractableBase` (single-file `.h/.cpp` under `Source/T66/Gameplay/`), following the existing subclass pattern (e.g. `T66CrateInteractable`). Wire it as:

1. **Interaction → fuse → explosion.** Override `Interact(APlayerController*)`. On first interact, guard against re-trigger (`bConsumed`/an `bArmed` flag, respecting `bShowcaseReusable`), then start a timer (`GetWorldTimerManager().SetTimer`, fuse seconds as an `EditAnywhere` C++ default — a narrow first-pass default is sanctioned by the contract). Return whatever the base's consume semantics expect. On timer expiry, run the explosion then hide/destroy (or reset if `bShowcaseReusable`).

2. **Damage both factions — do NOT route through the trap utils.** `FT66TrapDamageUtils` is hard-typed to `AT66TrapBase`, and the contract explicitly says TNT is not a trap, so it can't be reused. Instead call the same primitives those utils call:
   - Gather targets with a sphere overlap (`UKismetSystemLibrary::SphereOverlapActors` or a transient `USphereComponent`/`GetOverlappingActors`) at an `EditAnywhere` radius.
   - Heroes: `UT66RunStateSubsystem::ApplyDamage(Damage, this, FName("TNT"/...), this)` via `GameInstance->GetSubsystem<UT66RunStateSubsystem>()`.
   - Enemies: `Cast<AT66EnemyBase>(Target)->TakeDamageFromEnvironment(Damage, this)` (signature at `T66EnemyBase.h:129`).
   - Consider optional falloff and a self-only-once guard; flat radius damage is acceptable for a first pass.

3. **Lab Collector spawnability — two edit sites, both required.** Add the ID→class branch in `AT66GameMode::SpawnLabInteractable` (`T66GameMode_Lab.cpp:244-252`, e.g. `else if (InteractableID == FName(TEXT("TNT"))) ClassToSpawn = AT66TNTInteractable::StaticClass();` plus the include), and add the matching `AddSpawnCard(...OnSpawnInteractable(FName(TEXT("TNT")))...)` in `T66CollectorOverlayWidget.cpp` near lines 379–381. Missing either one leaves it unspawnable or uncompiled.

4. **Visuals.** Base already assigns a primitive cube; placeholder is in-scope, so no Pixal3D work. Optional tint/scale tweak only.

5. **Verification.** Run a focused module compile, then a staged standalone Lab run: spawn TNT via Collector, interact, confirm the fuse delay and that nearby heroes **and** enemies both take damage. If the runtime damage-to-both-factions path isn't actually observed in a staged run, call that out explicitly as unverified per the stop condition — compile-only is not sufficient here.

## Evidence Checked

- `T66WorldInteractableBase.h/.cpp` — base API: `Interact` is pure-virtual, `bConsumed`/`bShowcaseReusable`, cube visual default, registry + prompt lifecycle.
- `T66TrapDamageUtils.cpp` — damage primitives: hero `RunStateSubsystem->ApplyDamage`, enemy `TakeDamageFromEnvironment`; all entry points require `AT66TrapBase`.
- `T66EnemyBase.h:129` — `TakeDamageFromEnvironment` signature.
- `T66GameMode_Lab.cpp:238-253` — `SpawnLabInteractable` ID→class switch.
- `T66CollectorOverlayWidget.cpp:76-381` — spawn-card UI → `OnSpawnInteractable` → `SpawnLabInteractable`.
- `T66CrateInteractable.h` — minimal subclass shape.
- World/global routers — no rule against this; TNT is not world-generation scope.

## Questions Or Blockers

None that require the user. Tuning defaults (fuse seconds, radius, damage, falloff) are unspecified but the contract authorizes scoped C++ defaults, so Codex can pick sensible values and leave them `EditAnywhere`.

## Caveats

- Confirm the exact `UT66RunStateSubsystem::ApplyDamage` and `TakeDamageFromEnvironment` signatures/`DeliveryMethod` conventions at implementation time rather than copying the trap call verbatim — the trap path applies trap-specific scaling/`CanAffect` gates that TNT should not inherit.
- Friendly-fire to heroes is intended per the prompt ("damages everything around it, heroes and enemies"); make sure no global hero-damage guard silently no-ops it.
- The two Lab-integration edit sites are the most likely thing to be half-done; both must land for live testing.
- Result is OK on the assumption the staged runtime explosion check is actually executed; if it can't be run in this environment, that limitation must be surfaced, not papered over.

</validator_independent_answer>

<codex_draft>
# Codex Draft: TNT Interactable

Implemented a new non-trap world interactable named TNT.

Changed files:
- `Source/T66/Gameplay/T66TNTInteractable.h`
- `Source/T66/Gameplay/T66TNTInteractable.cpp`
- `Source/T66/Gameplay/GameMode/T66GameModePrivate.h`
- `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp`
- `Source/T66/UI/T66CollectorOverlayWidget.cpp`

Behavior:
- `AT66TNTInteractable` derives from `AT66WorldInteractableBase`, so it uses the existing generic world-interactable controller flow.
- Pressing interact lights the fuse once, marks the interactable consumed during the fuse, changes its primitive visual color, and starts a `FuseSeconds` timer.
- When the timer expires, TNT applies radial damage by actor-distance checks:
  - hero HP through `UT66RunStateSubsystem::ApplyDamage(DamageHP, this, "TNTExplosion", this)`;
  - rich enemies through `AT66EnemyBase::TakeDamageFromEnvironment`;
  - lightweight mobs through their existing `TakeDamageFromHeroHitZone` API using a body hit-zone handle and TNT source/event IDs;
  - awakened living bosses through their existing hit-zone damage API using a core hit-zone handle.
- Explosion also emits a debug damage sphere and reuses `UT66CombatComponent::SpawnDeathBurstAtLocation` for a simple explosion burst.
- Non-showcase TNT hides/disables/destroys after exploding. Showcase reusable TNT resets after the explosion.
- Prompt target is `TNT`; prompt action is `light fuse`.

Integration:
- Added TNT to `AT66GameMode::SpawnLabInteractable(FName)` under the `TNT` ID.
- Added a TNT card to the lab Collector interactables tab.

Verification:
- Focused editor build passed:
  `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
- First staged readiness attempt failed before smoke because an existing staged `T66.exe` locked `Saved\StagedBuilds\Windows\Engine\Binaries\ThirdParty\DbgHelp\dbghelp.dll`.
- Second staged readiness attempt reached the full wrapper:
  `Scripts\RunStagedBuildReadinessGate.ps1 -ClientConfig Development`
  - Stage: PASS
  - Staged executable: exists at `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - ProjectRoot shortcut: PASS
  - PinnedTaskbar shortcut: PASS
  - Smoke suite: FAIL, unrelated frontend gate `05_TopBarPowerUpNavigation` missing marker `Frontend automation: widget dump wrote`.
  - Evidence summary: `Saved\StagedBuildReadiness\20260607_162706\summary.json`

Known caveat:
- I did not add a dedicated automated TNT interaction smoke. Runtime proof currently covers compile, cook/stage, and existing smoke wrapper behavior, but not a log-asserted TNT fuse/damage event.

</codex_draft>
