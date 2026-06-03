# FullOperator Revision Prompt: Demo Gating Visibility Phase 3

You are Claude Operator for `C:\UE\T66`; Codex is Validator/Finisher.

Continue under the same approved scope:

`Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase3.md`

Validator result: REVISE.

Blocker:
- `Reports/AgentReviews/DemoGatingVisibility/phase3_completion_packet.md` claims
  `T66HeroGridScreen.cpp` and `T66CompanionGridScreen.cpp` use playable ID lists
  on both populate paths, but validation found the first population paths still
  call `GI->GetAllHeroIDs()` and `GI->GetAllCompanionIDs()`.
- Exact current anchors:
  - `Source/T66/UI/Screens/T66HeroGridScreen.cpp:101`
  - `Source/T66/UI/Screens/T66CompanionGridScreen.cpp:103`

Required revision:
1. Change those initial grid population paths to `GetPlayableHeroIDs()` and
   `GetPlayableCompanionIDs()` respectively.
2. Re-run the focused T66 compile.
3. Update `Reports/AgentReviews/DemoGatingVisibility/phase3_completion_packet.md`
   so it accurately describes the revised grid behavior and notes this revision.

Do not make unrelated changes. Do not use native goal tools. Do not commit, push,
tag, reset, clean, run broad Git/LFS scans, or inspect Unreal binary asset
folders.
