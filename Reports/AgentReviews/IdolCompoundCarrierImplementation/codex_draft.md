Working task:
Operator: Codex
Validator: Claude
Scope: Implement distinct basic-shape compound in-flight carrier recipes for all 20 idol projectiles, preserving one logical traveler for gameplay and preparing the profile namespace for per-rarity expansion.
Stop condition: Code/data/script changes are made, editor/staged verification is attempted, traveler visual proof is run, and any visual-proof gap is reported.

Codex draft result:
- Implemented 20 distinct idol carrier recipes in `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp`.
- Recipes expand one logical outgoing traveler into up to 4 primitive visual rows, preserving the existing logical traveler slot for gameplay, pooling, collision, and stats.
- Added rarity-ready profile normalization so future IDs like `TravelerVisual.Fire.AOE.Black`, `.Red`, `.Yellow`, and `.White` resolve to the current base recipe until rarity-specific recipes are authored.
- Added diagnostics for uploaded visual rows, peak visual rows, and carrier-recipe expansion.
- Updated `Source/T66/PerformanceSystem/T66OutgoingTravelerStressHarnessActor.*` to report used carrier recipe signatures and correct temporary colors, including purple electricity and gray wind.
- Updated `Scripts/RunOutgoingTravelerVisualProfilesGate.ps1` so the mixed 20-profile proof requires 20 unique visual profiles, 20 unique carrier recipe signatures, and visual-row expansion above the logical live count.

Verification performed:
- Editor build passed:
  `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- Staged standalone build passed:
  `C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
- Shortcut targets verified:
  `C:\UE\T66\T66 Standalone.lnk`
  `%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
  Both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Runtime visual profile gate passed:
  `C:\UE\T66\Saved\VideoCaptures\OutgoingTravelerVisualProfiles_20260605_120445_Readable\visual_profiles_gate_summary.json`
  Mixed case reported 20 visual profiles, 20 recipe signatures, peak logical live count 20, peak uploaded visual rows 74, no dropped travelers, and no failed spawns.
- Side-camera diagnostic gate also passed structurally:
  `C:\UE\T66\Saved\VideoCaptures\OutgoingTravelerVisualProfiles_20260605_120655_SideReadable\visual_profiles_gate_summary.json`
- No leftover Unreal/build processes were running after verification.

Known caveat:
- The runtime proof is strong structural proof that all 20 carrier recipes are used and expanded through the Niagara traveler pool, but the current proof camera does not give clean per-shape artistic inspection. The top-down captures compress the travelers into a narrow band, and the side-camera diagnostic missed the traveler row. This should be reported as PARTIAL for visual-art signoff, with a follow-up recommendation to add a dedicated carrier inspection camera or contact-sheet capture before final shape approval.

PPF close draft:
- Process used: user-approved temporary/basic-shape carrier workflow inside the existing Niagara outgoing-traveler pool.
- Matches declared process: YES for infrastructure and structural runtime proof; PARTIAL for visual-art signoff because the capture camera did not clearly frame each silhouette.
- Mechanisms present: one logical traveler preserved; per-idol carrier silhouette namespace implemented; rarity-ready profile IDs normalized; visual capacity scaled to recipe parts; structural runtime proof generated.
