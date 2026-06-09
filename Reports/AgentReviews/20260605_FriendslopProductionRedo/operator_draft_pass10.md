# Operator Draft: FriendslopStyle Main Menu Pass10

Status: NOT DONE / visual FAIL.

Codex produced a new runtime screen version after rejecting the manual crop text-covering method.

What changed:

- Replaced the bad crop-erasure artifact strategy with clean alpha-sheet target-size plates generated under:
  - `SourceAssets/UI/FriendslopStyle/MainMenu/*_round06.png`
  - `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/*_round06.png`
- Added explicit Round06 chrome identifiers in `Source/T66/UI/Style/T66FriendslopStyle.h/.cpp`.
- Wired top bar, left panel, center CTA/title, and right leaderboard to Round06-specific chrome in:
  - `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
  - `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
- Added `-T66FriendslopReferenceFixture` to force deterministic capture data for Solobro, ticket 53, one online friend, four offline friends, and one local leaderboard row.
- Refreshed the staged standalone build and shortcuts.

Proof:

- Editor build succeeded.
- `Scripts/StageStandaloneBuild.ps1` succeeded and refreshed `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Current capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_capture.png`
- Current dump: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_dump_utf8.json`
- Verifier report: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_fidelity.md`
- Contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_contact_sheet.png`
- Visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_visual_scorecard.md`

Verification result:

- `VerifyUIFidelity.py`: `PASS=249 FAIL=4 UNSURE=0`
- `Visual scorecard`: `Result: FAIL`

Main failures:

- The screen no longer has the rejected manual text-cover bars.
- Rows now fit inside their panels under the fixture.
- But the clean alpha-sheet chrome family is too pipe-like and heavy compared to the Round06 reference.
- Top bar, left panel, right panel, and CTA/button family still do not match Round06 at a glance.
- A pass11 inpaint experiment was attempted but rejected before capture because its plate contact sheet had warped ghost artifacts; pass10 clean-sheet assets were restored afterward.

Final answer should show the reference and produced capture in chat, and should not claim DONE.
