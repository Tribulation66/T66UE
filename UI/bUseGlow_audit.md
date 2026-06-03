# FT66ButtonParams bUseGlow Audit

Date: 2026-05-11

Source audited: `C:\UE\T66\Source\T66\UI\Style\T66Style.h` and all `FT66ButtonParams(` construction sites under `Source\T66\UI` / `Source\T66\Gameplay`.

## Decision

I did **not** flip `FT66ButtonParams::bUseGlow` to `false` globally. Existing non-migrated UI still has many `FT66ButtonParams` construction sites that do not make an explicit glow decision and are still routed through legacy chrome/reference helpers. A global default flip would be a broad visual change to screens outside the current migration scope.

Instead:

- `FT66ButtonParams` now tracks whether `SetUseGlow(...)` was called explicitly.
- `FT66Style::MakeButton()` logs a non-shipping warning when a button reaches the legacy helper with implicit `bUseGlow=true`.
- The warning text tells migration work to use `FT66FlatButtonParams` or `SetUseGlow(false)`, and tells legacy screens to opt in with `SetUseGlow(true)` when glow is intentional.

This reduces the silent failure surface without changing non-migrated screens' current visuals.

## Summary

| Category | Count | Migration meaning |
|---|---:|---|
| Explicit `SetUseGlow(false)` | 2 | Safe if the default flips later. |
| Settings wrapper disables glow | 13 | Already flattens params before rendering. |
| PowerUp generated/reference wrapper | 4 | Mostly routes through sprite/reference paths; needs screen-specific migration audit. |
| Hero Selection legacy helper path | 11 | Current flat `BuildSlateUI()` should not call these; keep out of reachable flat path. |
| Account reference chrome path | 8 | Legacy reference UI; do not flip globally before Account migration. |
| Other implicit legacy/default-risk sites | 43 | Must be reviewed or converted during each screen's migration. |

## Construction Site Audit

| Site | Classification | Source excerpt |
|---|---|---|
| `Source/T66/UI/Gambler/T66GamblerOverlayWidget_Build.cpp:1515` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(FText::GetEmpty(),` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:287` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(EquipText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:298` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:320` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(PreviewText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:330` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(EquipText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:340` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:359` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(PriceText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:421` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(RefundText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Skins.cpp:439` | Hero Selection legacy helper path; flat Build should not call it | `MakeHeroSelectionButton(FT66ButtonParams(PriceText,` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:26` | Hero Selection legacy helper path; flat Build should not call it | `FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:122` | Hero Selection legacy helper path; flat Build should not call it | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h:1167` | Hero Selection legacy helper path; flat Build should not call it | `return MakeHeroSelectionButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:38` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Label, MoveTemp(OnClicked), ET66ButtonType::Neutral)` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:343` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Private.h:702` | Settings wrapper disables glow before `MakeButton` | `return MakeSettingsButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_HUD.cpp:116` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Graphics.cpp:356` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Loc ? Loc->GetText_Apply() ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Crashing.cpp:41` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Loc ? Loc->GetText_ApplySafeModeSettings() ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Crashing.cpp:73` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Loc ? Loc->GetText_ReportBug() ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp:21` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Text, FOnClicked::CreateLambda(...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp:79` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(RebindText, FOnClicked::CreateLambda(...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp:91` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(ClearText, FOnClicked::CreateLambda(...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Controls.cpp:268` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(Loc ? Loc->GetText_RestoreDefaults() ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:51` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(TabDefinition.Label, ...` |
| `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:103` | Settings wrapper disables glow before `MakeButton` | `FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), ...` |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1566` | PowerUp generated/reference wrapper; no global default flip before screen test | `FT66ButtonParams(ButtonText, ...` |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1757` | PowerUp generated/reference wrapper; no global default flip before screen test | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1892` | PowerUp generated/reference wrapper; no global default flip before screen test | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp:2027` | PowerUp generated/reference wrapper; no global default flip before screen test | `FT66ButtonParams(TabText, MoveTemp(OnClicked), ...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:1376` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(Label, ...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:1731` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2025` | Explicit false; safe | `FT66ButtonParams(FText::GetEmpty(), ...).SetUseGlow(false)` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2279` | Explicit false; safe | `FT66ButtonParams(FText::GetEmpty(), ...).SetUseGlow(false)` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2334` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2373` | Account reference chrome; relies on legacy visual path | `MakeAccountReferenceButton(FT66ButtonParams(...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2581` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(NSLOCTEXT("T66.Account", "OpenAppeal", "APPEAL"), ...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2629` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(SubmitAppealButtonText, ...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2640` | Account reference chrome; relies on legacy visual path | `FT66ButtonParams(NSLOCTEXT("T66.Account", "CancelAppeal", "CANCEL"), ...` |
| `Source/T66/UI/Screens/T66AccountStatusScreen.cpp:2748` | Account reference chrome; relies on legacy visual path | `MakeAccountReferenceButton(FT66ButtonParams(BackText, ...` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:842` | Companion Selection reference chrome; likely relies on legacy visual path | `MakeCompanionReferenceButton(FT66ButtonParams(EquipText,` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:878` | Companion Selection reference chrome; likely relies on legacy visual path | `MakeCompanionReferenceButton(FT66ButtonParams(PreviewText,` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:896` | Companion Selection reference chrome; likely relies on legacy visual path | `MakeCompanionReferenceButton(FT66ButtonParams(BuyText,` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:921` | Companion Selection reference chrome; likely relies on legacy visual path | `MakeCompanionReferenceButton(FT66ButtonParams(EquipText,` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1248` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1302` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1327` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1825` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1883` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1897` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1917` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1997` | Companion Selection legacy path; review during migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66CompanionGridScreen.cpp:519` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(FText::GetEmpty(), ...` |
| `Source/T66/UI/Screens/T66AchievementsScreen.cpp:1000` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(Label, ...` |
| `Source/T66/UI/Screens/T66AchievementsScreen.cpp:1346` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66AchievementsScreen.cpp:1469` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66ChallengesScreen.cpp:614` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(FText::GetEmpty(), OnClicked, ...` |
| `Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp:258` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(Label, OnClicked, ...` |
| `Source/T66/UI/Screens/T66PartyInviteModal.cpp:451` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66PartyInviteModal.cpp:465` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(` |
| `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp:254` | Implicit true legacy risk; review during screen migration | `MakeMasterLibraryButton(FT66ButtonParams(StayText, ...` |
| `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp:260` | Implicit true legacy risk; review during screen migration | `MakeMasterLibraryButton(FT66ButtonParams(QuitText, ...` |
| `Source/T66/UI/Screens/T66HeroGridScreen.cpp:315` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(FText::GetEmpty(), TileClicked)` |
| `Source/T66/UI/Screens/T66RunSummaryScreen.cpp:748` | RunSummary sprite/reference wrapper; fallback still needs migration review | `FT66ButtonParams(Label, OnClicked, ...` |
| `Source/T66/UI/Screens/T66LanguageSelectScreen.cpp:591` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(ConfirmText, ...` |
| `Source/T66/UI/Screens/T66ReportBugScreen.cpp:352` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(SubmitText, ...` |
| `Source/T66/UI/Screens/T66ReportBugScreen.cpp:364` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(CancelText, ...` |
| `Source/T66/UI/Screens/T66SavePreviewScreen.cpp:261` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(BackText, ...` |
| `Source/T66/UI/Screens/T66SavePreviewScreen.cpp:268` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(LoadText, ...` |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp:1037` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(PreviewText, ...` |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp:1047` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(LoadText, ...` |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp:1093` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(BackText, ...` |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp:1159` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(PrevText, ...` |
| `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp:1170` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(NextText, ...` |
| `Source/T66/UI/T66CasinoShopTabWidget.cpp:626` | Implicit true legacy risk; review during screen migration | `FT66ButtonParams(` |

## Follow-Up Rule

During each Stage 2 screen migration, any `FT66ButtonParams` site in that screen's reachable code must either be converted to `FT66FlatButtonParams` / `FT66FlatStyle`, explicitly set `SetUseGlow(false)`, or be recorded as intentionally retained legacy chrome before that screen can pass Step 0.
