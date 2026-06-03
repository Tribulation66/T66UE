Verdict: REVISE

## Blockers

None.

## Major Issues

- **Enum-switch consumer audit is incomplete.** The Pass 4 grep covers iteration patterns (`NumEnums`, `TEnumRange`, `TArray<ET66HeroStatType>`, `StaticEnum<...>`), but does not cover `switch (...PrimaryStatType)` / `case ET66HeroStatType::` consumers. Appending `Special` to `ET66HeroStatType` will silently add a value that exhaustive switches with `default: ensureMsgf(false, ...)` or `checkNoEntry()` could trip on. Add a grep for `case ET66HeroStatType::` and `switch.*PrimaryStat` (excluding Mini) before declaring the leak risk closed.
- **Runtime stat-application path for `Special` is unaddressed.** The plan rewires UI to render `Special` as a category, but does not state what happens when the value-rolling / stat-apply pipeline encounters `PrimaryStatType == Special` with a non-zero `MainValue` (which automation via `T66GameplayAutoPickupCard` can produce). At minimum, the plan should identify the apply/roll function(s), confirm whether `Special` is a no-op there, and either short-circuit rolling for `Special` items or document why a stray rolled `MainValue` is harmless.
- **Gambler's Token consistency between synthetic and CSV is not verified.** Only `T66GameInstance.cpp`'s synthetic `Item_GamblersToken` row is touched. If `Content/Data/Items.csv` (or another data asset) also has a Gambler's Token row with `PrimaryStatType=Luck`, that row needs to flip to `Special` too, or the two sources will disagree at runtime. The packet should explicitly state whether a CSV row exists and, if so, include it in Planned Production Edits.

## Minor Issues

- The exact player-visible string for the `Special` category line is not quoted. "Special" as a literal label may be acceptable, but the plan should pin down the display string (and confirm `GetPrimaryStatLabel()` returns it via `LOCTEXT` and not a raw `FString`) so the verification screenshot has something concrete to confirm.
- Save-compatibility step (7) falls back to "report no fixture available and cite source-level missing-row skip proof." That fallback is fine, but the packet should also state where in source the legacy-row skip is implemented so the citation is unambiguous at write-up time.
- The pending-issue stub for shared `Content/Items/Sprites/Item_HpRegen_*.uasset` / `Item_LifeSteal_*.uasset` cleanup should record the specific reason it is deferred (Mini-inclusive ownership pass not yet done) so future-reader does not re-open it without that context.

## Clarifying Questions

- Does `Content/Data/Items.csv` (or any other data asset) carry an `Item_GamblersToken` row in addition to the synthetic? If so, is changing it to `Special` in this pass intended?
- Is `Special` ever expected to participate in value-rolling, or should the roller short-circuit to `MainValue = 0` for `Special` primaries?
- Is `T66ItemCardTextUtils::GetPrimaryStatLabel()` the only call site that maps `PrimaryStatType` to display text, or is there a parallel mapping (e.g., in `T66CollectorOverlayWidget` or a HUD widget) that also needs the new case?

## Required Verification

In addition to what is listed in the packet:

- Add a non-Mini grep for `case ET66HeroStatType::` and `switch.*PrimaryStat` (and `EnumIndex` / direct comparisons against named enumerators) and confirm every consumer either handles `Special` or has a safe default.
- During the staged Quick Revive automation launch (`-T66GameplayAutoPickupCard=Item_BackroomsQuickRevive ...`), explicitly check the log for `Ensure failed` / `LogOutputDevice: Warning: ... ensureMsgf` lines mentioning `PrimaryStatType` or `HeroStatType`, not just crash/assert/launch failure.
- Confirm via grep that no `.uasset` outside `Content/Data` references `Item_HpRegen` / `Item_LifeSteal` by ID string (e.g., a Blueprint hard-coding the row name); if such a reference exists, the deferred-asset pending issue must capture it.
- Confirm the CSV-validation script asserts `PrimaryStatType` parses cleanly into the new enum (i.e., the CSV's `Special` token matches the `UMETA(DisplayName=...)` import name UE expects, not just the display name).

## Rationale

Pass 4 substantively closes the Pass 3 enum-pool concern by showing real stat pools are explicit arrays, justifies the art-pipeline edit through ownership routing in `Tools/README.md`, and gives `Special` a sensible non-numeric card treatment. Scope discipline (no Mini edits, deferred shared-sprite cleanup tracked in `Content/Data/pending_issues_Data.md`) is appropriate. The remaining gaps are: (1) the enum audit was framed around iteration, not switch/case, leaving exhaustive-switch consumers unverified; (2) the value-rolling/stat-apply path for `Special` is not characterized; (3) Gambler's Token consistency between the synthetic and any CSV row is unstated. Those are tractable with a focused additional audit and one or two clarifications, so the plan is REVISE rather than BLOCK — but not yet APPROVE-safe to present at the AGENTS.md go-ahead gate.

