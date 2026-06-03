Codex Approval: APPROVE

## Approved Task

Undeprecate the Minigames top-bar entry and minigame screen family while leaving Arcade deprecated. Move Minigames from the deprecated inventory to demo-gated invisible content so the current forced-demo build still hides Minigames.

## Approved Scope

- `Config/DefaultGame.ini`
- `Source/T66/Core/T66DeprecatedFeatureSettings.h`
- `Source/T66/Core/T66DeprecatedFeatureSettings.cpp`
- `Source/T66/UI/T66UIManagerReleaseVariant.cpp`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Screens/T66MinigamesScreen.cpp`
- `Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp`
- `Source/T66/Core/T66DirectEntry.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
- `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`
- `Demo/DEPRECATED_CONTENT.md`
- `Demo/DEMO_RELEASE_INSTRUCTIONS.md`
- `Reports/AgentReviews/UndeprecateMinigamesDemoGate/*`

## Approved Tool Surface

Claude FullOperator may read/edit approved text source, config, docs, run focused Unreal compile/build commands, run staged standalone refresh if needed, run Unreal-owned UI capture/dump scripts for Main Menu and Minigames screen proof, and write a completion packet under `Reports/AgentReviews/UndeprecateMinigamesDemoGate/`.

## Required Process Rules

- Follow root `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Demo/DEMO_AGENTS.md`, `UI/UI_AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Minigames/MINIGAMES_AGENTS.md`, and `Reports/AGENTS.md`.
- Do not fork the demo or delete full-game rows.
- Keep minigame modes isolated; do not edit mode-owned Mini/TD/Deck/Idle gameplay implementations unless a compile blocker proves one adjacent include/name fix is required.
- Keep Arcade deprecated: do not set `bDisableArcadeGames=false`, do not set `bDisableArcadeInteractables=false`, and do not alter arcade runtime availability except where a Minigames container still references an arcade/Versus card.
- Demo-gated content should be hidden/blocked, not visibly overlaid as `COMING SOON`, when reachable from current demo UI.

## Explicitly Excluded Actions

- No Git commit, push, branch, reset, clean, checkout, or broad Git/LFS scans.
- No edits under `Content/`, `SourceAssets/`, staged build output, Steam upload tooling, backend repo, or arcade runtime implementation.
- No enabling Arcade games/interactables.
- No cosmetic redesign of the top bar beyond the smallest layout change needed to make the Minigames tab available outside demo and absent in demo.

## Verification Required After Operator Run

- Focused compile/build for touched C++.
- If source/config changes affect standalone behavior, run `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` and verify the staged exe path and shortcuts.
- Produce Unreal-owned proof that current demo Main Menu/top bar still does not show Minigames and that direct Minigames screen access is blocked or absent in demo.
- If practical, also produce a non-demo direct-entry proof using `-T66FullGame` showing the Minigames screen/top-bar entry can exist outside demo. If this is impractical, state exactly why.
- Completion packet must list files changed, verification commands, pass/fail markers, caveats, and Claude token data from the helper manifest when available.

## Approval Rationale

The user made a direct implementation request with explicit scope: Minigames should move from deprecated to demo-gated, Arcade must remain deprecated, and the demo should still hide Minigames. The approved scope is bounded to the central deprecated-feature setting, release-variant screen gate, top-bar/navigation seams, minigame container availability checks, and the two inventory docs.
