# Pending Issues - RunState

## Resolved: Staged Standalone Build Blocked By Undeclared Vendor Token Item ID [Resolved - Blocker]

- Severity tag: [Blocker]
- What's wrong: `Scripts\StageStandaloneBuild.ps1` failed during the Win64 `T66` target build on 2026-05-28 because `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` referenced an undeclared legacy token item ID constant.
- Resolution: The item taxonomy pass canonicalized the runtime item ID as `Item_VendorToken`, retained legacy `Item_GamblersToken` alias handling only for compatibility, and removed the undeclared constant reference from RunState economy inventory code.
- Follow-up: Rerun `Scripts\StageStandaloneBuild.ps1` as part of the current item taxonomy verification pass.

## Resolved: Staged standalone build blocked by undeclared `LogT66RunState` [Resolved - Blocker]

What's wrong: `Scripts\StageStandaloneBuild.ps1` fails during the build step because `Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp` logs through `LogT66RunState` in `ApplyAutomationHeroHPOverride`, but that log category is not declared in the compile unit or an included header.

Resolution: `T66RunStateSubsystem_Combat.cpp` now defines `LogT66RunState`, and the B.10.1D Resume3 focused Development build plus standalone stage completed successfully.

Follow-up: None for this compile blocker.

## Resolved: Automation hero HP override is capped below ranged-active measurement need [Resolved - Blocker]

What's wrong: `UT66RunStateSubsystem::ApplyAutomationHeroHPOverride` clamps `T66.AutoCapture.HeroHPOverride` to `1000.0`. The B.10.1D reviewed HP2000 acceptance rerun parsed and recorded `T66AutoCaptureHeroHPOverride=2000`, but the run-state log showed `AppliedHP=1000.0 RequestedHP=2000.0 MaxHP=1000.0 CurrentHP=1000.0`. The stationary ranged-active measurement contract therefore cannot currently apply the reviewed survival margin.

Resolution: B.10.1D Resume3 raised the automation-only cap to `2000.f`. B.10.1D Resume4 then raised it to `50000.f` with an explicit automation-only measurement comment. Staged HP20000 smoke and acceptance logs confirmed `AutoCaptureHeroHPOverride AppliedHP=20000.0 RequestedHP=20000.0 MaxHP=20000.0 CurrentHP=20000.0`, and no HP20000 row killed the hero.

Follow-up: The HP cap issue is closed. Gameplay acceptance still blocks because CVar-on route validity can report rich Ranged spawns/fire attempts under lightweight routing; that remaining issue is tracked in `Source\T66\Gameplay\pending_issues_Gameplay.md`.
