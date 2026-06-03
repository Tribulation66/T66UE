Phase 1 complete. All work stayed inside the approved scope.

## Summary

**Files changed (3, all approved):**
1. `Config/DefaultDemoMode.ini` — `bAllowDrugPurchases=true`; `MaxDiplomaUpgradesPerStat=4`
2. `Source/T66/Core/T66BuffSubsystem.cpp` — `AreSingleUseBuffPurchasesAllowed()` now delegates to `UT66ReleaseVariantSubsystem::AreDrugPurchasesAllowed()` (header already included; mirrored the existing local subsystem-fetch pattern)
3. `Source/T66/UI/Screens/T66AchievementsScreen.cpp` — `bDemoAchievementRowsLocked` set to `false`, neutralizing both the Steam and Secret coming-soon overlay loops

**Verification:**
- Focused compile `Build.bat T66Editor Win64 Development … -WaitMutex -NoHotReloadFromIDE` → **`Result: Succeeded`** (22.2s incremental; recompiled the two affected unity chunks, relinked `UnrealEditor-T66.dll`)
- Confirmed against source: `MaxFillStepsPerStat = 4`, and both gates read exactly the two config fields I edited (`IsDiplomaUpgradeAllowed` uses `MaxDiplomaUpgradesPerStat`; `AreDrugPurchasesAllowed` returns `bAllowDrugPurchases`)
- UI capture **deferred** to the later combined UI visibility proof phase (per approval), with reason recorded

**Completion packet:** `Reports/AgentReviews/DemoGatingVisibility/phase1_completion_packet.md`

No commit/push/tag, no Mini/minigame or out-of-scope changes. Token ledger marked Unavailable (no helper manifest). This is an Operator artifact for Codex to validate and finalize.
