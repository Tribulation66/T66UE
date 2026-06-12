# Demo Gating Visibility — Phase 1 Completion Packet

Operator: Claude (FullOperator)
Validator: Codex
Date: 2026-05-30
Approval: `Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase1.md`

## Outcome

PASS (Operator work artifact — not a greenlight). All three Phase 1 seams were
changed inside the approved scope and a focused editor compile succeeded. Drugs,
diploma upgrades (full fill-step cap), and Steam/Secret achievement rows are now
out of demo/coming-soon locking.

## Files Changed

1. `Config/DefaultDemoMode.ini`
   - `bAllowDrugPurchases=false` → `bAllowDrugPurchases=true`
   - `MaxDiplomaUpgradesPerStat=1` → `MaxDiplomaUpgradesPerStat=4`
2. `Source/T66/Core/T66BuffSubsystem.cpp`
   - `UT66BuffSubsystem::AreSingleUseBuffPurchasesAllowed()` hard `return false;`
     replaced with delegation to
     `UT66ReleaseVariantSubsystem::AreDrugPurchasesAllowed()`.
3. `Source/T66/UI/Screens/T66AchievementsScreen.cpp`
   - `bDemoAchievementRowsLocked` initializer changed from
     `T66DemoModeUI::IsDemoModeActive(this)` to `false`, neutralizing both the
     Steam (line ~1379) and Secret (line ~1759) coming-soon overlay loops.

## Code-Level Proof

### 1. Drug purchases now delegate to the central release-variant gate
`Source/T66/Core/T66BuffSubsystem.cpp`:
```cpp
bool UT66BuffSubsystem::AreSingleUseBuffPurchasesAllowed() const
{
    if (const UGameInstance* GI = GetGameInstance())
    {
        if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GI->GetSubsystem<UT66ReleaseVariantSubsystem>())
        {
            return ReleaseVariant->AreDrugPurchasesAllowed();
        }
    }

    return false;
}
```
The include `#include "Core/T66ReleaseVariantSubsystem.h"` was already present at
the top of the file (line 6); no new header was required. This mirrors the
existing local pattern in `IsDemoDiplomaUpgradeLimitReached()` in the same file.

### 2. Demo config enables drug purchases
`Config/DefaultDemoMode.ini` now contains `bAllowDrugPurchases=true`.
`UT66ReleaseVariantSubsystem::AreDrugPurchasesAllowed()` returns
`Settings->bAllowDrugPurchases` for Steam demo builds (and `true` otherwise), so
the purchase path is now allowed in demo.

### 3. Demo diploma cap set to the full `MaxFillStepsPerStat` value
`Source/T66/Core/T66BuffSubsystem.h` defines
`static constexpr int32 MaxFillStepsPerStat = 4;` (verified from source before
editing). `Config/DefaultDemoMode.ini` now sets `MaxDiplomaUpgradesPerStat=4`.
`UT66ReleaseVariantSubsystem::IsDiplomaUpgradeAllowed()` compares
`CurrentUnlockedSteps < MaxDemoUpgrades` (MaxDemoUpgrades from that setting), so
all 4 fill steps are now reachable in demo.

### 4. Achievement row overlays no longer activate from demo mode
`Source/T66/UI/Screens/T66AchievementsScreen.cpp`:
```cpp
const bool bDemoAchievementRowsLocked = false;
```
This is the sole gate guarding both `if (bDemoAchievementRowsLocked)` blocks that
wrap Steam and Secret rows with `T66DemoModeUI::WrapWithComingSoonOverlay(...)`.
With the gate constant-false, neither coming-soon overlay loop executes in any
mode. The overlay-construction code is left intact for the later UI phase.

## Verification

### Focused C++ compile — PASS
Command:
```
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
```
Result marker: `Result: Succeeded` (Total execution time: 22.18 seconds).
Incremental build recompiled the two affected unity chunks
(`Module.T66.23.cpp`, `Module.T66.34.cpp`) and relinked `UnrealEditor-T66.dll`.

### UI capture/dump — DEFERRED
Deferred to the later combined UI visibility proof phase (per approval lines
48–49 and the Phase 3 UI scope). Reason: the three changes are
config/logic-gated demo behaviors whose visible effect (drug-purchase
availability on PowerUp, a 4-step diploma cap, and absence of coming-soon
overlays on Steam/Secret achievement rows) requires standing up a demo-mode
editor/standalone session and navigating multiple screens — the exact scope of
the combined UI proof phase. Capturing in isolation now would duplicate that
work and is not required to prove the Phase 1 code/config seams, which are
proven above at the code level and by a clean compile.

## Token Ledger

- Claude token count: Unavailable (no helper manifest exposing a Claude token
  count was used for this run).

## Caveats

- Demo-mode gating only diverges from full-game behavior when
  `IsSteamDemoBuild()` is true; in non-demo builds `AreDrugPurchasesAllowed()`
  and `IsDiplomaUpgradeAllowed()` already returned permissive values, so these
  edits change demo behavior specifically.
- The neutralized `bDemoAchievementRowsLocked` leaves two now-unreachable
  overlay blocks in place intentionally, so the row-overlay rendering code
  remains available for the later UI hiding/visibility phase rather than being
  deleted in Phase 1.
- No commit/push/tag was made; no Mini/minigame, deprecated-inventory, or other
  out-of-scope code was touched.
- This artifact is an Operator work product; Codex validates the actual changes
  and authors the final user-facing report.
