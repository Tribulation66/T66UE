# Movement And Run Summary Equivalence

Date: 2026-05-27

This note closes the Gate A proof item for the removed movement-speed wrapper and the old Run Summary line.

## Code-Level Equivalence

- Pre-cleanup `UT66RunStateSubsystem::GetMovementSpeedSecondaryMultiplier()` returned literal `1.f`.
- Pre-cleanup `UT66HeroMovementComponent::RefreshMovementSpeedFromRunState()` multiplied the movement stack by `CachedRunState->GetMovementSpeedSecondaryMultiplier()`.
- Removing that factor removes a literal `* 1.f` multiplier from the active movement formula. The remaining movement stack still applies:
  - speed-stat resolved base walk speed
  - item movement multiplier
  - stage movement multiplier
  - status movement multiplier
- Packaged Hero Movement QA verified the active result after removal in `hero_movement_qa_verification.json`: stable `MaxWalkSpeed=840.0`, max velocity `840.0`, max delta speed `886.2`, forward displacement `5406.0`, and jump Z movement.

## Run Summary Display

- Pre-cleanup `UT66RunSummaryScreen::BuildSlateUI()` rendered `Move Speed Mult` using `FormatMultiplier(RunState->GetMovementSpeedSecondaryMultiplier())`.
- Because that wrapper always returned `1.f`, the old line was a legacy display-only `1.0x` row, not a distinct active stat source.
- The cleanup intentionally removed that legacy `Move Speed Mult` row instead of replacing it.
- Packaged direct Run Summary smoke verified the screen still renders:
  - screenshot: `Reports/Hygiene/2026-05-27/run_summary_smoke.png`
  - dump: `Reports/Hygiene/2026-05-27/run_summary_smoke_dump.json`
  - verification: `Reports/Hygiene/2026-05-27/run_summary_smoke_verification.json`
- `run_summary_smoke_verification.json` confirms the dump contains `RUN SUMMARY` labels and zero `Move Speed Mult` labels.

## Scope Note

This is not a claim that the removed movement wrappers had no source callsites before deletion. They did have live source callsites. The approved Gate A work removed a no-op movement factor and its legacy Run Summary row, then verified movement runtime behavior with the existing Hero Movement QA route.
