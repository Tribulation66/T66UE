You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_LevelUpStatsBreakdown\completion_report_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Completion Report Review Packet

Working goal: Finish the item/stat pipeline change so items only affect secondary stats, level-up/XP/diploma/drug stat infrastructure is wired data-driven, and the user receives the final item/stat breakdown plus verification evidence.

Reviewer role: Read-only reviewer. Check whether this completion report is accurate, whether any implementation caveat should block reporting completion, and whether the verification evidence supports the claims. Do not propose unrelated refactors.

Applicable instructions:
- Root `AGENTS.md` requires Claude review for substantive output, staged standalone verification for playable runtime changes, and exact verification reporting.
- Default scope excludes Mini/minigame systems unless explicitly named. The user did not ask for Mini work and later said not to worry about Mini.
- HP regen and lifesteal items should not be presented in the user-facing item breakdown.

Implementation summary to report:
- Normal item rows now only affect their secondary stat line. Item primary stats no longer add run primary stats or primary-derived secondary bonuses.
- `Item_HpRegen` and `Item_LifeSteal` are removed from `Content/Data/Items.csv`; their old IDs are rejected by the inventory smoke path. Dormant runtime secondary enum/functions remain for compatibility and return zero/default where applicable.
- `Item_BackroomsQuickRevive` and `Item_VendorToken` use primary category `Special`; canonical item name is Vendor Token.
- Accuracy/Evasion/Armor/Luck order in `Content/Data/Items.csv` is:
  - Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance
  - Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance
  - Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance
  - Luck: Loot Crate, Loot Chest, Loot Bag, Loot Wheel
- Execute, Assassinate, and Crush use a shared non-boss OHKO rule: regular enemies and minibosses can be killed, bosses cannot.
- Loot Bag and Loot Wheel secondaries improve their matching interactable reward rarity flow, same concept as Loot Crate and Loot Chest.
- Level-up is enabled and data driven:
  - `Content/Data/PlayerExperience.json` now has `LevelUpXPThreshold` and `LevelUpWaveRadiusUU`.
  - Current data uses flat threshold `100` and wave radius `900`.
  - `Content/Data/Enemies.csv` now has `XPValue`; current data uses `20` per production row.
  - Both rich enemies and lightweight mobs grant XP on hero-credit kills.
  - Level-up heals to full, rolls hero primary gains from `Heroes.csv` per-level gain weights, applies primary-to-secondary propagation, kills nearby non-boss enemies/mobs inside the level-up wave, and suppresses wave-kill XP so it does not chain.
- Diplomas are wired to actual permanent primary stat bonuses from unlocked fill steps and random overflow; those primary bonuses propagate into secondary stats. Diploma purchases remain unavailable/gated.
- Drugs are wired so selected owned single-use buffs apply secondary multipliers at run start; purchases remain unavailable/gated.

Known caveat:
- Old shared `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset` files still exist. This is documented in `Content/Data/pending_issues_Data.md` as requiring a Mini-inclusive ownership/reference audit before deletion. Since this task excluded Mini scope, the completion report should identify this as a caveat, not silently claim every asset with those names was deleted.

Verification evidence:
- Prior implementation review greenlight:
  - `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/20260528T220313-pass2/claude_review_pass2.md`
  - First line was `Verdict: APPROVE`.
- Compile:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - Result: succeeded. Only noted warning was pre-existing Niagara deprecation in `T66Hero1AxeAOEVFXLabActor.cpp`.
- Data reloads:
  - `Scripts\SetupPlayerExperienceDataTable.py`: exit 0.
  - `Scripts\SetupCombatRosterDataTables.py`: exit 0.
- Editor stat smoke:
  - `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/stat_pipeline_smoke_editor.json`
  - `ok: true`; checks included secondary-only items, diploma primary/secondary propagation, drug secondary multiplier, XP level-up, full heal, primary gain, secondary propagation, rich and lightweight enemy XP, and no XP chain from level-up wave.
- Editor item taxonomy smoke:
  - `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/item_taxonomy_smoke_editor.json`
  - `ok: true`; checks included Execute/Assassinate/Crush reject bosses and allow minibosses, Vendor Token alias compatibility, Loot Bag/Loot Wheel reward improvement, and retired item IDs skip inventory.
- Staged standalone:
  - `Scripts\StageStandaloneBuild.ps1`: succeeded.
  - Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Both `C:\UE\T66\T66 Standalone.lnk` and the taskbar pinned `T66 Standalone.lnk` target that staged exe.
- Staged stat smoke:
  - `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/stat_pipeline_smoke_staged.json`
  - `ok: true`.
- Staged item taxonomy smoke:
  - `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/item_taxonomy_smoke_staged.json`
  - `ok: true`.
- `git diff --check` on the touched files returned no whitespace errors, only line-ending warnings.

Requested final answer shape:
- Keep concise.
- Include the item breakdown in this order:
  - Weapon Modifiers: Damage, Attack Speed, Attack Scale
  - Character Modifiers: Accuracy, Evasion, Armor
  - Luck Modifier: Luck
  - Special Items: Item Quick Revive, Vendor Token
- Include short summaries for Level Up, Diplomas, and Drugs.
- State that the stat wiring is verified and note the caveat about leftover HP regen/lifesteal sprite assets.

</review_packet>
