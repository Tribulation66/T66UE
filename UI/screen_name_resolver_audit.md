# T66 Frontend Screen Name Resolver Audit

Date: 2026-05-11

Source audited: `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp`

Capture entry point audited: `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`

## Resolver Behavior

`TryResolveFrontendScreenName()` compares names with `ESearchCase::IgnoreCase`, so runtime matching is case-insensitive. The spellings below are the canonical spellings as written in the resolver and should be used in docs, pass logs, scripts, and capture invocations.

After this audit, an unknown `-T66FrontendScreen=<name>` logs `Error` and requests process exit with status `66`. `CaptureT66UIScreen.ps1` also scans the staged log for `Frontend automation: unknown screen override` and fails with a nonzero exit instead of accepting any PNG produced by a fallback/default screen.

## Accepted Screen Names

| Canonical | Aliases | ET66ScreenType |
|---|---|---|
| MainMenu |  | MainMenu |
| HeroSelection | HeroSelect | HeroSelection |
| SaveSlots | SaveSlot | SaveSlots |
| CompanionSelection | CompanionSelect | CompanionSelection |
| Settings | SettingsScreen | Settings |
| LanguageSelect | Language | LanguageSelect |
| Achievements |  | Achievements |
| Minigames |  | Minigames |
| PauseMenu | Pause | PauseMenu |
| ReportBug |  | ReportBug |
| RunSummary |  | RunSummary |
| PowerUp |  | PowerUp |
| HeroGrid |  | HeroGrid |
| CompanionGrid |  | CompanionGrid |
| QuitConfirmation | Quit | QuitConfirmation |
| PartyInvite |  | PartyInvite |
| AccountStatus | Account | AccountStatus |
| PlayerSummaryPicker | SummaryPicker | PlayerSummaryPicker |
| SavePreview |  | SavePreview |
| MiniMainMenu |  | MiniMainMenu |
| MiniCharacterSelect |  | MiniCharacterSelect |
| MiniCompanionSelect |  | MiniCompanionSelect |
| MiniDifficultySelect |  | MiniDifficultySelect |
| MiniIdolSelect |  | MiniIdolSelect |
| MiniSaveSlots |  | MiniSaveSlots |
| MiniShop |  | MiniShop |
| MiniRunSummary |  | MiniRunSummary |
| TDMainMenu |  | TDMainMenu |
| TDDifficultySelect |  | TDDifficultySelect |
| TDBattle |  | TDBattle |
| IdleMainMenu | IdleChadpocalypse | IdleMainMenu |
| DeckMainMenu | Deckbuilder, ChadpocalypseDeckbuilder | DeckMainMenu |
| VersusMainMenu | Versus, ChadpocalypseVersus | VersusMainMenu |
| Challenges |  | Challenges |
| DailyDescent |  | DailyDescent |

## Capture Invocation Audit

| Source | Name used | Resolver status | Notes |
|---|---|---|---|
| `Saved\Codex\UI\HeroSelection\pass_log.md` pass 01-07 | HeroSelection | OK | Canonical. |
| `Saved\Codex\UI\HeroSelection\pass_log.md` regression note | Overview | MISMATCH | Not accepted by resolver; caused false Overview regression capture. Use `AccountStatus` to open the Account screen, then add a dedicated account-tab automation flag if a specific Overview/History tab must be forced. |
| `UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md` Stage 1 capture example | HeroSelection | OK | Canonical. |
| `Audit\Reference\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` example | DailyDescent | OK | Canonical. |
| `UI\Instructions\UI_SCREEN_WORKFLOW_INSTRUCTIONS.md` placeholder | `<ScreenKey>` | Template | Must be replaced with one of the accepted canonical names above. |
| `UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md` placeholder | `<ScreenName>` | Template | Must be replaced with one of the accepted canonical names above. |

## Master Plan Screen List Comparison

| Master plan label | Capture screen name to use | Status / Notes |
|---|---|---|
| Hero Selection | HeroSelection | OK. |
| Overview | AccountStatus | Label mismatch. `Overview` is an Account tab, not a resolver screen. Need a tab-forcing automation path before using it as an isolated regression target. |
| History | AccountStatus | Label mismatch. `History` is an Account tab, not a resolver screen. Needs the same tab-forcing automation path. |
| Diplomas | PowerUp | Label mismatch. `Diplomas` is a Power Up tab/category, not a resolver screen. |
| Drugs | PowerUp | Label mismatch. `Drugs` is a Power Up tab/category, not a resolver screen. |
| Steam Achievements | Achievements | Label mismatch. `Steam` is an Achievements tab/category, not a resolver screen. |
| Minigames | Minigames | OK. |
| Settings -> Retro FX | Settings | Label mismatch only for the tab; resolver screen is `Settings`. |
| Daily Descent | DailyDescent | OK. |
| Challenges | Challenges | OK. |
| Load Game | SaveSlots | Label mismatch. Resolver does not accept `LoadGame`; use `SaveSlots` or add an alias deliberately. |
| Run Summary | RunSummary | OK. |
| Main Menu | MainMenu | OK. |
| Pause Menu | PauseMenu | OK. |
| Quit Confirmation | QuitConfirmation | OK. |
| Report Bug | ReportBug | OK. |
| Hero Grid | HeroGrid | OK. |
| Companion Grid | CompanionGrid | OK. |
| Language Select | LanguageSelect | OK. |
| Party Invite | PartyInvite | OK. |
| Account Status | AccountStatus | OK. |
| Player Summary Picker | PlayerSummaryPicker | OK. |
| Save Preview | SavePreview | OK. |
| Companion Selection | CompanionSelection | OK. |

## Mismatches to Fix in Future Loop Docs or Automation

| Mismatch | Risk | Recommended fix |
|---|---|---|
| `Overview` used as `-Screen Overview` | Captures Main Menu or fails after this change; cannot represent Overview tab. | Use `-Screen AccountStatus` and add a separate `-T66AccountTab=Overview` automation path if needed. |
| `History` as a screen label | Same as Overview. | Use `AccountStatus` plus a tab override. |
| `Load Game` in plan | Human label does not match resolver. | Use `SaveSlots` in capture commands, or add a `LoadGame` alias intentionally. |
| Power Up sub-screens `Diplomas` / `Drugs` | Human labels do not match resolver. | Use `PowerUp` plus a future tab/category override. |
| Achievements sub-screen `Steam` | Human label does not match resolver. | Use `Achievements` plus a future tab/category override. |
