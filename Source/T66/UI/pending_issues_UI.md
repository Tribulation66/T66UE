# Pending Issues - UI

## Casino Overlay Tab Switch Destroys Gambler Round State

- Severity tag: [Major]
- What's wrong: `UT66CasinoOverlayWidget::OpenGamblerTab` schedules `FT66FlatStyle::DeferRebuild`, whose RemoveFromParent path runs the overlay's `NativeDestruct`, which calls `SharedOverlay::RemoveFromParentAndReset(CasinoGamblerTabWidget)` and nulls the tab. The next rebuild creates a fresh gambler tab, wiping `RoundState`, `LockedGamePage`, `bCasinoSessionShouldConsumeOnClose`, and any locked/lost wager. In Full casino mode a player who loses a round can switch Vendor -> Gambler to get a fresh tab and keep gambling, dodging the lost-round close-only lock and the consume-on-close report. Discovered while building the 2026-06-10 casino capture automation (the automation had to late-arm on the post-rebuild tab instance; see `[CasinoCapture] late-arm` in `T66PlayerController_Overlays.cpp`).
- Why it's out of scope now: The 2026-06-10 pass was scoped to the four gambler game visuals/animations plus capture automation. Moving casino session state out of the tab widget (or making the tab survive overlay rebuilds) is a separate ownership/lifecycle change with anti-cheat reporting implications.
- What fixing it would entail: Either persist the gambler session state outside the widget (e.g., a small struct on the player controller or RunState passed back in on tab creation), or stop resetting the tab in `NativeDestruct`/`DeferRebuild`, then re-verify the double-down automation proof, consume-on-close reporting, and the tab-switch flow in Full casino mode.

## Resolved: FriendslopStyle Standard Modal Button Slice Integrity At 300x58 [Major]

- Resolution: Pass 04 regenerated the red, green, and dark standard modal button plates as exact 300 x 58 textless PNGs through the account-backed imagegen worker process, then switched the shared modal button helper to Slate `Image` rendering with `FMargin(0)`. This removes the seam-prone 9-slice cap budget at the accepted 300 x 58 runtime size.
- Evidence: Worker validation passed at `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass04_workers\standard_modal_buttons_exact_300x58\validation.json`; runtime captures passed visual inspection at `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass04_captures_20260608\quit_confirmation.png`, `party_invite.png`, and `save_preview.png`.
- Remaining note: Future variable-width modal button variants must not reuse this fixed-image spec. Rerun the slice/contact-sheet gate and generate new size-specific plates or implement a proved slice mode before changing modal button width.

## Casino Alchemy And Report Bug Dead Code Needs Full Archive Cleanup

- Severity tag: [Major]
- What's wrong: The live UI routes for Casino Alchemy and Report Bug were retired for the overlay capture pass, but compiled dead code and historical localization/checklist references remain. Casino Alchemy still has unused helper/widget code in the casino overlay area and older localization/data references; Report Bug still has its standalone screen source, enum/localization entries, and historical fidelity docs even though active entry points were removed.
- Why it's out of scope now: The current pass was scoped to removing the active overlay/capture routes, adding tooltip capture support, and producing the requested modal/tooltip screenshots. A full archive cleanup would cross UI, localization, data/save compatibility, and possibly backend/report-output ownership.
- What fixing it would entail: Decide whether these systems should be hard-deleted or archived as compatibility-only, remove or move the dead source/docs/localization rows under the owning process, refresh generated localization/assets as needed, and run a focused compile plus staged UI smoke for the affected frontend/gameplay routes.

## Transient Slate Tooltip Capture Lacks Structured Dump Support

- Severity tag: [Minor]
- What's wrong: The FriendslopStyle tooltip automation can open and screenshot a transient Slate tooltip widget added directly to the viewport, but `T66AutoDumpWidget` does not currently dump that widget. The dump-target resolver searches active viewport `UUserWidget` roots and does not match the raw Slate tooltip content tag, so the tooltip primitive fit gate can only use screenshot/log evidence today.
- Why it's out of scope now: The current pass was scoped to generating/integrating standard modal button chrome, correcting shared modal/tooltip sizing, and producing requested captures. Extending the dump resolver to index transient Slate viewport content is a separate UI diagnostics feature.
- What fixing it would entail: Add a repo-owned dump path for transient viewport Slate widgets, either by registering the tooltip overlay as a named capture root or teaching `T66WidgetDumpTargets` to inspect viewport-added Slate content by metadata tag, then rerun `CaptureT66UIWidget.ps1` with `-T66GameplayAutoCapture=tooltip` and require a JSON dump for the tooltip primitive fit gate.

## Resolved: Frontend Top Bar Icon Action Builder No Longer Blocks Standalone Stage [Blocker]

- Resolution: A fresh Development standalone stage succeeded on 2026-06-08 after the `MakeIconActionButton` call-signature mismatch was fixed in `Source/T66/UI/T66FrontendTopBarWidget.cpp`. The refreshed staged executable was written to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, and the session loaded-travel smoke passed afterward at `C:\UE\T66\Saved\AgentReviews\SessionLoadedTravelStageRerun\session_loaded_travel_smoke_after_stage\summary.json`.
- Remaining note: Keep this entry as historical context only. Future top-bar compile failures should get a new pending issue with the current compiler error signature and proof path.

## Archived Mods And Retro FX Checklist Rows Need Regeneration

- Severity tag: [Minor]
- What's wrong: Several historical UI geometry/checklist documents still contain row-level assertions for archived controls such as `HeroSelection.BottomRow.ModsButton`, `CompanionSelection.ModsButton`, and Settings `RetroFXButton` entries.
- Why it's out of scope now: This pass deprecated the live UI entry points, routing, and high-authority references. Regenerating every fidelity checklist/geometry table is a separate documentation-maintenance pass.
- What fixing it would entail: Re-run the UI fidelity checklist/geometry generation for Hero Selection, Companion Selection, and Settings tabs, then archive or rewrite the old row assertions so future validation does not expect removed controls.

## Frontend Screens Lack A Central Controller Focus Contract

- Severity tag: [Major]
- What's wrong: Direct-entry automation can open frontend screens without mouse clicks, but the UI layer does not yet expose a central per-screen controller-focus contract for Steam Deck validation. Individual screens can still vary in their initial focus, directional navigation, accept, and back behavior.
- Why it's out of scope now: This pass builds deterministic screen/run access and avoids mouse automation. It does not retrofit every screen's gamepad focus behavior or add a full controller-navigation smoke matrix.
- What fixing it would entail: Add a focus contract/helper for native and WBP-backed screens, define expected first-focus and navigation rules per screen, then add staged automation that launches each target through direct entry and validates gamepad toggle/accept/back behavior.

## Frontend Settings Tag-Click Smoke Exits Before Screenshot

- Severity tag: [Major]
- What's wrong: `Scripts\RunStagedBuildReadinessGate.ps1` refreshed the staged Development build on 2026-06-08, but the pre-release smoke suite failed in `01_FrontendTagClick` because `04_TopBarSettingsNavigation` exited before its screenshot was created. The first three top-bar cases passed, and the gate summary is at `C:\UE\T66\Saved\StagedBuildReadiness\20260608_052217\summary.json`.
- Why it's out of scope now: The current pass is scoped to the gameplay HUD reward presentation queue. The failing smoke is frontend top-bar/settings navigation and does not exercise the reward queue path.
- What fixing it would entail: Re-run `RunFrontendTagClickSmokeMatrix.ps1` for the Settings case, inspect `C:\UE\T66\Saved\StagedBuildReadiness\20260608_052217\smoke_suite\01_FrontendTagClick\04_TopBarSettingsNavigation\run.log`, determine why the staged game exits before screenshot capture, and fix the frontend automation/navigation path.

## Loot Wheel Boost Rewards Lack A Focused Result Toast

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66LootWheelInteractable.cpp` can lock and commit Gold, Item, or Boost results, but the HUD currently has focused presentation lanes only for gold chest rewards and item pickup cards. Boost results can be committed idempotently after the world spin, but there is no generic stat-boost result card/toast to show after landing.
- Why it's out of scope now: Phase 3 is constrained to applying the animation infrastructure and preserving the existing HUD presentation controller ownership. Adding a new generic reward presentation surface would be a broader UI feature.
- What fixing it would entail: Add a queued generic reward/toast lane to `FT66HUDPresentationController`, support stat-boost title/body/icon data, and route loot wheel boost results through that lane after the landing marker.

## FriendslopStyle Iteration Runtime Color Regressions

- Severity tag: [Major]
- What's wrong: The pass21 Main Menu output regressed some runtime colors: the Load Game button plate reads purple instead of the reference dark/black plate, and the Invite button no longer reads as a strong green action button.
- Why it's out of scope now: The current follow-up pass is explicitly font-only. It should not regenerate image assets or modify button-color assets until the font fix is reviewed.
- What fixing it would entail: In the next FriendslopStyle iteration, inspect whether the regression came from imagegen prompt output, runtime brush selection, tint, or slice/stretch behavior; then regenerate or replace the affected asset family and verify in a capture/contact sheet.

## FriendslopStyle Iteration Time Needs A Parallel Worker Pass

- Severity tag: [Major]
- What's wrong: The full pass21 Main Menu iteration took about 1 hour 20 minutes, with avoidable serial wait time around family worker execution, packaging review, staging, and cross-review.
- Why it's out of scope now: The current follow-up pass is scoped to fixing the font only, not redesigning the full iteration pipeline.
- What fixing it would entail: Add a documented fast-path plan for launching all failed-family Codex CLI imagegen workers in parallel, normalizing worker manifests, reducing duplicate contact-sheet/report generation, and reserving full staging/capture for the end of the pass.
