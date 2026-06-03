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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\Hero1LegacyIdolAliasAudit\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\Hero1LegacyIdolAliasAudit\codex_completion_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\Hero1LegacyIdolAliasAudit\20260603T060602-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

Do the save proof audit so we can delete the legacy aliases and move on. Other than that its good go.

# Task Contract

Working task:
Operator: Codex unless the repo state says otherwise.
Validator: configured non-Operator if available.
Scope: audit save/proof/content references for legacy idol aliases, decide whether alias deletion is safe, and if safe make the scoped alias-removal/normalization changes needed to proceed with the temporary Hero 1/idol visual plan. Mini/minigames excluded.
Stop condition: report the audit result, any changes made, and current verification evidence.

# Relevant Repo Rules

- Root router: `AGENTS.md`.
- Role protocol: `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Current role state: `.t66/operator-state.json` says Codex operator and Claude validator.
- Do not use native goal tools.
- Default scope excludes Mini/minigames.
- Runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect playable standalone.
- Data asset import/reload work must run the owning Unreal commandlet/import script when source CSV/JSON changes affect DataTables.
- Do not claim current proof that was not run.

# Current Plan Context

Previous planning packet accepted:
`Reports/AgentReviews/Hero1TempShapesImplementationPlan/codex_draft.md`

The relevant plan step is to remove legacy idol aliases from authored temporary visual keys and runtime visual lookup, while keeping compatibility mappings only if live save/proof/content audit proves they are still needed.

# Validator Request

Provide an independent repo-grounded answer for this exact request:

1. What should be audited before deleting legacy idol aliases?
2. Which files/systems are most likely to contain save, proof, data, or runtime dependencies on aliases such as `Idol_Water`, `Idol_Storm`, `Idol_Electric`, `Idol_Light`, `Idol_Poison`, `Idol_Lava`, etc.?
3. If the audit finds no required dependencies, what scoped implementation is safe?
4. If it finds dependencies, what should be changed first before deleting aliases?
5. What verification should Codex run?

Return `Result: OK` if this can be handled internally. Return `Result: NEEDS_USER` only if the user must choose or approve a blocker.

</original_prompt>

<validator_independent_answer>
I have enough evidence to give the independent answer.

Result: OK

## Independent Answer

The request is authorized and internally executable by Codex — the user already said "good go," and whether aliases are safe to delete is an evidence question, not a user decision. But the audit must distinguish two different things that share the same words, because conflating them will break content.

**1. What must be audited before deleting legacy idol aliases**
- The `NormalizeLegacyIdolID()` map in `Source/T66/Core/T66IdolManagerSubsystem.cpp:131-164` (legacy element IDs → new category IDs like `Idol_Ice_AOE`).
- Every raw `IdolID == FName(TEXT("Idol_<element>"))` comparison still compiled in runtime (VFX, localization, combat).
- Save round-trip: what idol IDs are actually persisted in save games and whether old saves still carry legacy IDs that only the normalize map rescues.
- Proof harness expectations: the `.ps1` proof runners and the diagnostic log vocabulary.
- **Critical:** sprite *asset* names in `Content/Data/Idols.csv` (`Idol_Water_black`, `Idol_Storm_red`, `Idol_Light_yellow`, `Idol_Poison_*`, `Idol_Lava_*`, etc.) are texture asset paths in the Icon columns — NOT idol IDs. They are live and referenced by current rows keyed on new IDs. Deleting/renaming those sprites is a separate, content-breaking action and must not be swept into "alias deletion."

**2. Files/systems most likely to hold dependencies**
- `Source/T66/Core/T66IdolManagerSubsystem.cpp:131-164` — the legacy normalize map itself (the alias source of truth).
- `Source/T66/Core/T66LocalizationSubsystem.cpp:1705-1775` — idol **names and tooltips are keyed entirely on legacy element IDs** (`Idol_Water`, `Idol_Storm`, `Idol_Light`, `Idol_Poison`, `Idol_Lava`, etc.). This is the largest live dependency; deleting aliases without re-keying loses all idol display text.
- `Source/T66/Gameplay/T66CombatVFX.cpp:444-467` — a legacy-keyed VFX fallback branch (`IdolID == Idol_Lava/Water/Light/...` and the `Idol_Storm` BP_Storm path).
- `Source/T66/Gameplay/T66CombatComponent.cpp:3586-3752` — legacy-element status branches, **but gated behind `bLegacyIdolSpecificStatusEnabled = false` with an early return at line 3565-3568**, so these are currently dead code (not a live runtime dependency, but still a deletion target to clean).
- `Content/Data/Idols.csv` Icon columns — sprite assets named after the aliases (live; see caveat above).
- `Scripts/RunHero1Axe*IdolImpactProof.ps1` and the `SourceID=Idol_Water` diagnostic strings (`T66CombatComponent.cpp:4035,4765`) — proof harness parses these; preserved intentionally.
- Save/backend: `T66BackendSubsystem.cpp:175-179` and overlay `EquippedIdols` use **new** category IDs, which is good evidence authored/current content has already migrated.

**3. If audit finds no required dependencies — safe scoped implementation**
The evidence already shows this is unlikely to be the case for localization/VFX. But where a key truly has zero live readers, the safe scoped change is: remove that authored alias key and its single runtime lookup branch, leaving the new category ID as the only path — without touching sprite asset names or proof diagnostic strings.

**4. If dependencies exist (they do) — what to change first**
Re-key before deleting. Order:
1. Migrate localization name/tooltip lookups to new category IDs (or route them through normalize) so no display text is lost.
2. Fold the legacy VFX fallback branch into the normalized lookup already present above it.
3. Keep `NormalizeLegacyIdolID` until a save audit proves no shipped/old save carries legacy IDs; the map is the back-compat safety net for persisted data.
4. Only then delete the now-unreferenced alias authoring keys. Leave proof diagnostic vocabulary and sprite asset names alone unless separately scoped.

**5. Verification Codex should run**
- C++ compile/build of the T66 module after any source edit (runtime-facing).
- The save round-trip proof the user explicitly asked for: load a save written with legacy IDs, confirm idols resolve, equip/run, re-save — capture the log.
- The existing `RunHero1Axe*IdolImpactProof.ps1` proof runners; confirm the `Idol_Water`/category diagnostic lines still PASS.
- If `Idols.csv` is touched, run the owning DataTable import commandlet (repo rule).
- Staged standalone validation if anything in the playable runtime path changes.

## Evidence Checked
- `Content/Data/Idols.csv` — rows keyed on new IDs (`Idol_Fire_DOT`…); alias words appear only as sprite asset paths in Icon columns.
- `T66IdolManagerSubsystem.cpp:131-164` — legacy→new normalize map.
- `T66LocalizationSubsystem.cpp:1705-1775` — names/tooltips keyed on legacy IDs (live).
- `T66CombatVFX.cpp:438-467` — mixed normalized + legacy-keyed VFX lookups.
- `T66CombatComponent.cpp:3564-3752, 4035, 4765` — legacy status branches dead-gated; diagnostic strings preserved.
- `T66BackendSubsystem.cpp`, `T66PlayerController_Overlays.cpp`, `T66IdolAltar.h`, `T66TutorialManager.cpp` — use new category IDs.
- Proof scripts `RunHero1AxeIdolCategoryNativeImpactProof.ps1`, `RunHero1AxeAOECategoryIdolImpactProof.ps1`.

## Questions Or Blockers
None requiring the user. The user has authorized the audit-and-scoped-change work. Codex can run it internally.

## Caveats
- No save/proof run has been executed in this validation; do not report deletion as proven safe until the save round-trip and proof runners actually run and pass.
- The single biggest risk is conflating the legacy **idol-ID aliases** (deletable after re-keying) with the **sprite asset names** of the same spelling in `Idols.csv` (live content — must not be deleted under this task).
- Localization is fully keyed on legacy IDs; a blanket alias deletion without re-keying will silently drop idol names/tooltips.
- Keep `NormalizeLegacyIdolID` until the save audit proves no persisted legacy IDs remain — it is the back-compat net, not dead weight.
- Mini/minigames excluded per scope; do not extend the sweep there.

</validator_independent_answer>

<codex_draft>
Result: OK

Operator: Codex
Validator: Claude
Scope: Non-Mini legacy idol alias save audit and scoped removal before Hero 1 temporary visual-shape work.

Summary:
- The save audit found no legacy idol aliases in active saves under `Saved/SaveGames` or `Saved/StagedBuilds/Windows/T66/Saved/SaveGames`.
- Archived non-Mini staged backups under `Saved/StageBackups` still contain legacy IDs such as `Idol_Light` in `EquippedIdols` and `DamageBySource`, so a total deletion of compatibility aliases is not safe.
- I kept legacy compatibility at the save/backend migration boundary and removed duplicated runtime/localization/VFX fallback branches.

Files changed in this pass:
- `Source/T66/Core/T66SaveMigration.h`
  - Added `T66MigrateLegacyIdolID`.
  - Updated `T66NormalizeEquippedIdolSaveArrays` to canonicalize legacy equipped idol IDs.
- `Source/T66/Core/T66IdolManagerSubsystem.cpp`
  - Routed `NormalizeLegacyIdolID` through `T66MigrateLegacyIdolID`.
  - Changed canonical electricity idol color to purple. Ice remains light blue.
- `Source/T66/Core/T66LocalizationSubsystem.cpp`
  - Removed legacy-specific idol name/tooltip branches.
  - Localization now normalizes the ID and uses canonical category suffix text.
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Removed legacy imported-VFX fallback branches and the explicit `Idol_Storm` BP fallback.
  - Canonical normalized IDs still route to imported temporary VFX paths.
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - Removed the disabled legacy-id status-effect branch and replaced it with a no-op local lambda.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Added save proof checks for direct legacy equipped-idol migration and backend JSON parse migration.
- `Reports/AgentReviews/Hero1LegacyIdolAliasAudit/codex_audit_and_change_summary.md`
  - Captures the audit result, allowed remaining aliases, and proof paths.

Allowed remaining legacy strings:
- `Source/T66/Core/T66SaveMigration.h`: migration table.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: proof inputs only.
- `Source/T66/Gameplay/T66CombatComponent.cpp` and `Source/T66/Gameplay/T66CombatVFX.cpp`: historical `Idol_Water` proof/diagnostic labels.
- `Content/Data/Idols.csv`: sprite asset paths, not idol IDs.
- Existing docs/review packets.

Verification:
- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
  - First attempt failed before compile due to `ConflictingInstance` UnrealBuildTool mutex.
  - Process audit showed no live UBT/dotnet/AutomationTool/editor/game process except the tray app.
  - Second attempt succeeded with `BUILD SUCCESSFUL`.
  - Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Root and taskbar shortcuts were refreshed to that staged executable.
- Staged save proof:
  - Ran staged `T66.exe` with `-T66Entry=Run:Tower -T66GameplayAutoCapture=runsummaryroundtrip -T66RunSummaryRoundTripProof=C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.json`.
  - Exit code `0`.
  - Manifest `C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.json` has `ok=true`.
  - New checks passed:
    - `Legacy idol save IDs migrate to canonical IDs`: `Idol_Light, Idol_Water, Idol_Storm, Idol_Poison` -> `Idol_Electricity_Pierce, Idol_Ice_AOE, Idol_Electricity_AOE, Idol_Nature_DOT`.
    - `Legacy idol backend IDs parse as canonical IDs`: same canonical result through `T66BackendRunSummaryParser`.
  - Log marker: `FPlatformMisc::RequestExitWithStatus(0, 0, T66RunSummaryRoundTripComplete)`.

Caveats:
- I did not rename or delete `Content/Data/Idols.csv` sprite asset paths with old idol words because those are current icon asset references.
- I did not clean historical docs/proof labels because the requested deletion target was runtime alias support, not proof vocabulary/doc history.
- I did not run Hero1 VFX impact proof scripts because this pass changed alias/save migration and staged run-summary proof, not the current VFX capture mechanism.

</codex_draft>
