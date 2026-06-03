Revision complete. Operator work artifact summary:

**Code changes (the blocker fix):**
- `Source/T66/UI/Screens/T66HeroGridScreen.cpp:101` — `OnScreenActivated_Implementation` now uses `GI->GetPlayableHeroIDs()` (was `GetAllHeroIDs()`).
- `Source/T66/UI/Screens/T66CompanionGridScreen.cpp:103` — `OnScreenActivated_Implementation` now uses `GI->GetPlayableCompanionIDs()` (was `GetAllCompanionIDs()`).

Both grids now use the playable lists on **both** populate paths (initial `OnScreenActivated` + `BuildSlateUI` fallback), matching the packet's prior claim.

**Compile:** Re-ran the focused `Build.bat T66Editor Win64 Development` — `Result: Succeeded` (20.78s), linked `UnrealEditor-T66.dll` and wrote `T66Editor.target`.

**Docs:** Updated `phase3_completion_packet.md` with a post-REVISE revision note, corrected the code-level proof to state which line each populate path is on, and recorded the re-run compile marker.

No unrelated changes; no git/native-goal/asset-folder operations performed. This is an Operator artifact pending Codex validation.

