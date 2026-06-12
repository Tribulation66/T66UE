Verdict: REVISE

Blockers
- None.

Major Issues
- Enum placement assumption is unverified. "Append `Special UMETA(DisplayName = "Special")` after `Accuracy` … so existing serialized enum values are preserved" only holds if `Accuracy` is currently the trailing entry in `ET66HeroStatType`. The packet does not show this; if any value (including a sentinel/Max) sits after `Accuracy`, the insertion point shifts subsequent serialized indices and silently corrupts old saves/data assets that store enum-as-byte. Confirm by reading the enum block in `Source/T66/Data/T66DataTypes.h` and append at the true tail.
- Smoke-test hedge contradicts the standing rule. The packet states staged standalone verification applies, identifies the staged exe path (`C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`), and then preemptively allows "rely on build + DataTable reload + static pool proofs" if the helper "provides that path." The path is already known and stable. Commit to launching and smoking the staged exe (main menu load + entering a run). Only allow the fallback if the launch itself fails, not as a planned-skip clause.

Minor Issues
- Save-compatibility claim is argued, not verified. "Missing rows are skipped" is plausible, but the packet provides no concrete proof for `Item_HpRegen`/`Item_LifeSteal` slot IDs after deletion. A single legacy-save boot test would convert this from inference to evidence.
- UI label coverage may be incomplete. `T66ItemCardTextUtils.cpp` is updated and `T66CollectorOverlayWidget` is covered via enum DisplayName, but the packet does not affirmatively rule out other label/tooltip/inventory-header surfaces that might branch on `ET66HeroStatType` without a default. A targeted re-grep on `PrimaryStatType` UI consumers would close the loop.
- Pipeline script change risk. Removing the Armor/HpRegen and Evasion/LifeSteal sheet series from `T66ProcessReimaginedItemSheets.py` is the right direction; confirm no orchestrator/batch JSON/CSV or sibling pipeline script still references those series by name so they are not accidentally re-emitted later.
- Pending-issues entry scope. The Pass 2 packet promises a Data pending-issue note about shared sprites preserved for Mini. Specify that the entry also names the affected sprite directories so future cleanup work can act on it without re-auditing.

Clarifying Questions
- Is `Accuracy` currently the last entry in `ET66HeroStatType` (no `Count`/`Max`/`MAX` sentinel and no later additions)? If not, what is the true tail to append after?
- Do any save files in source control, CI fixtures, or your local test-saves library still carry `Item_HpRegen`/`Item_LifeSteal` inventory rows that the legacy-save boot test should exercise?
- Are there any non-`Items.csv` data assets (e.g., shop/drop tables, loot manifests, achievement unlock lists) referencing `Item_HpRegen`/`Item_LifeSteal` that the deletion audit might have missed because they live outside `Source/Content/Gameplay/Scripts/Tools`?

Required Verification
- Read `ET66HeroStatType` definition and append `Special` at the actual tail; record the line context in the implementation report.
- Build `T66Editor Win64 Development` (UE 5.7) — must succeed without new warnings on stat-enum switches.
- Run `Scripts/SetupItemsDataTable.py` via `UnrealEditor-Cmd.exe`; confirm `DT_Items` saves without import failures and re-run the post-reload CSV checks.
- Static post-edit grep: no main-run `Item_HpRegen`/`Item_LifeSteal` outside updated historical doc and the new pending-issue entry; `Item_BackroomsQuickRevive.PrimaryStatType == Special`; `T66GameInstance.cpp` synthetic `Item_GamblersToken.PrimaryStatType == Special`; random-pool exclusions unchanged.
- Run `Scripts/StageStandaloneBuild.ps1`; launch the staged `T66.exe` from the documented path; smoke main menu and entering a run; verify taskbar shortcut target unchanged.
- Legacy-save boot: load a save containing `Item_HpRegen` and/or `Item_LifeSteal` slot IDs (or hand-edited equivalent); confirm no crash, no warning spam, and no stat application from the missing rows.

Rationale
The audit work is solid and scope discipline is good — Mini correctly excluded, shared sprites correctly preserved with a tracked pending issue, enum-vs-mechanics separation handled, random-pool gates respected, and the right consumer paths inspected. The plan is close to greenlit. Two issues prevent immediate approval: the "append after Accuracy preserves serialization" claim is asserted without showing the current enum tail (a real risk for save/data integrity if `Accuracy` is not last), and the staged-standalone smoke step is pre-hedged in a way that conflicts with the standing rule when the exe path is already known. The save-compat reasoning is plausible but deserves a concrete boot test rather than only an argument. Tighten these three points in Pass 3 and the plan should be safe to present at the AGENTS.md go-ahead gate.

