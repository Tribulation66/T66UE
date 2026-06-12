# FullOperator Prompt: Demo Gating Visibility Phase 4

You are Claude Operator for `C:\UE\T66`; Codex is Validator/Finisher.

Use FullOperator mode only inside the approved proof/staging scope from:

`Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase4.md`

Do not use native goal tools. Do not edit source/config/docs. Do not commit,
push, tag, reset, clean, run broad Git/LFS scans, or inspect Unreal binary asset
folders outside normal staging/capture outputs.

Working task:
Operator: Claude
Validator: Codex
Scope: Phase 4 only. Refresh staged standalone, verify shortcut target, capture
or dump UI proof, and write completion packet.
Stop condition: stage/shortcut/capture proof is produced or exact blockers are
recorded, and completion packet is written.

Required proof root:
`Reports/Proof/DemoGatingVisibility/2026-05-30-phase4`

Create `.report-run.json` there with `expiresAfterDays: 15`.

Recommended command sequence:
1. Run `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`.
2. Verify staged exe exists:
   `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
3. Verify shortcut target for:
   - `C:\UE\T66\T66 Standalone.lnk`
   - `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` if present.
4. Use Unreal-owned captures/dumps through `Scripts/CaptureT66UIScreen.ps1` or
   `Scripts/CaptureT66UIWidget.ps1` against the staged exe. Prefer both screenshot
   and dump when practical.

Screens/proof to attempt:
- `MainMenu`
- `HeroSelection`
- `Diplomas` (PowerUp tab)
- `Drugs` (PowerUp tab)
- `SteamAchievements`
- `Achievements` with secret tab if there is a supported command-line tab arg
  (for example extra arg `-T66AchievementsTab=Secret` if supported)

Use extra args to force demo mode if needed. The current config has
`bForceDemoMode=true`, but if capture output suggests non-demo, rerun with the
existing demo command-line flag used by the project.

Proof checks:
- Preserve screenshots/dumps/log paths.
- Check dumps/logs for `COMING SOON`, `DemoOverlay`, `DailyDescentButton`, and
  relevant tags where dumps are available.
- Visually inspect screenshots enough to report whether proof is FULL or PARTIAL.
- Specifically call out whether hero carousel physical slots repeat playable
  heroes due to existing wraparound. If repeated playable heroes are visible,
  classify it as a caveat and do not claim the carousel reduced to exactly five
  physical boxes.

Write:
`Reports/AgentReviews/DemoGatingVisibility/phase4_completion_packet.md`

Completion packet must include:
- Outcome FULL/PARTIAL/BLOCKED
- Staging command and pass/fail marker
- Shortcut target verification
- Screenshot/dump/log artifact paths
- Findings for each requested screen
- Any skipped proof and exact reason
- Token ledger with Claude token count if exposed by helper manifest or otherwise
  `Unavailable`
- Caveats
