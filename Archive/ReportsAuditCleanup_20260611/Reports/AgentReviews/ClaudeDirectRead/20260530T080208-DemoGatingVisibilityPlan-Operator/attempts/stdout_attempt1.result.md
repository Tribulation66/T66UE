I have the seams identified. One note before the packet: I'm honoring the read-only profile (no file writes), so the packet is delivered inline for Codex to validate and save.

---

# Operator Packet: Demo Gating Visibility

## 1. Working Task And Validation Depth

**Task contract**
- **Operator:** Claude (read-only this phase)
- **Validator/Finisher:** Codex
- **Scope:** Invert the demo "COMING SOON" concept. (a) Move drugs, diploma upgrades, and achievements (Steam + secret) out of the demo-gated bucket into *available* content. (b) Author two Markdown inventories — demo-gated-but-invisible content, and deprecated content. (c) Hide remaining demo-gated UI entries (extra hero-carousel boxes, non-Easy difficulties, Daily Descent / Lab buttons) entirely instead of showing the locked overlay, keeping backend/data authoring intact for easy re-enable.
- **Stop condition (this phase):** seams + anchors identified, folder instructions read, bounded phase plan + concrete first mutating phase proposed for Codex approve/reject without redesign.

**Validation depth:** Full. Spans runtime UI (C++ Slate), config (`.ini`), behavioral gate logic, docs, and proof-bearing capture verification.

**Tier:** Broad, multi-system, proof-bearing → **phase-bounded**; mutating phases route to FullOperator (per Proof-Bearing Work Routing).

**Scope boundaries:** Frontend gate/visibility only. No backend leaderboard/account work. Mini/minigame/arcade/casino systems are **out of scope for changes** (see §3).

## 2. Roles And Tool Profile

- **Operator model:** Claude (`claude-opus-4-8`).
- **Validator model:** Codex.
- **Finisher:** Codex (final user-facing report).
- **Helper/profile used now:** `Scripts\Invoke-ClaudeReadOnlyOperator.ps1` (ReadOnly, non-mutating, no approval artifact required). This packet is **not a greenlight**.
- **Codex approval artifact for mutating phases:** `Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval.md` (first line exactly `Codex Approval: APPROVE`) — does not yet exist; required before any `-ToolProfile FullOperator` run.
- **Operator run dir / manifest:** read-only run; manifest path n/a in this conversation surface.

## 3. User Constraints And Out Of Scope

**Explicit user constraints**
- Remove visible COMING SOON; show real content again.
- Demo-gated content becomes **hidden from the visible game**, existing in backend/data only, **easy to re-enable later**.
- Hero carousel shows only owned heroes; difficulty box shows only `Easy`.
- Daily Descent / Lab demo overlays removed (buttons hidden, not overlaid).
- **Move to available:** drugs, diploma upgrades, achievements (Steam + secret).
- Two **separate** `.md` files: demo-gated-invisible vs deprecated.
- User-ordered sequence: (1) change what's locked/unlocked, (2) write both lists, (3) hide demo-gated entries.

**Implementation approval status:** Planning only this phase. No edits made.

**Mini/minigame status:** Out of scope for changes. The COMING SOON consumers in `T66VersusArcadeScreen.cpp:277`, `T66MinigamesScreen.cpp:280`, `T66ArcadeSelectionWidget.cpp`, `T66CasinoGamblerTabWidget_Build.cpp` are **not touched**. They are relevant only as *entries in the deprecated inventory doc* (see Open Decision D2).

**Out of scope:** backend Steam-ticket/leaderboard policy; arcade/casino/minigame code; any DataTable cook/import (no DataTable source changes are planned).

## 4. Applicable Instructions Read

| Path | Why it applies | One-line takeaway |
|---|---|---|
| `AGENTS.md` | Root router | Derive task contract; phase broad work; PPF only if process-governed; staged-standalone rule for playable changes. |
| `OPERATOR_VALIDATOR_PROTOCOL.md` | Operator/Validator stack active (`.t66/operator-state.json`) | Produce complete packet; proof-bearing → FullOperator after Codex approval; phase-bounded. |
| `.t66/operator-state.json` | Role source | Operator=Claude, Validator=Codex, Global. |
| `Demo/DEMO_AGENTS.md` | Demo content gates owned here | "Use the central release/content gate"; don't fork project; don't delete full-game rows. |
| `Demo/DEMO_RELEASE_INSTRUCTIONS.md` | Demo gate authority | **Conflict:** line 100-101 mandates COMING SOON overlay *instead of hiding* — user request inverts this; doc must be updated as part of the work. |
| `UI/UI_AGENTS.md` | UI Slate screens touched | Fidelity-loop only required for reference-image work; no live labels baked into art. |
| `Reports/AGENTS.md` | Artifact routing | Packets under `Reports/AgentReviews/<TaskSlug>/`; durable docs are not raw runs. |
| `UI/Checklists/pending_issues_Checklists.md` | Only pending_issues found in candidate folders | (No conflicting prior decision on demo gating; confirm during Phase 1.) |

Backend agents not read in depth: no backend authority change is in scope (frontend visibility only).

## 5. Evidence And Live Findings

### Central gate model
- `Source\T66\Core\T66ReleaseVariantSubsystem.h:71-120` / `.cpp` — `UT66ReleaseVariantSubsystem` exposes `IsHeroAllowed`, `FilterHeroIDs`, `IsDifficultyAllowed`, `GetVisibleDifficulties` (returns **all** difficulties, `.cpp:324-327`), `GetPlayableDifficulties` (Easy in demo, `.cpp:297-322`), `IsRunCategoryAllowed` (Lab gated by `bAllowLabRun`, `.cpp:220-234`), `IsDiplomaUpgradeAllowed` (`.cpp:345-355`), `AreDrugPurchasesAllowed` (`.cpp:357-366`), `GetUnavailableContentText`.
- `Source\T66\UI\T66DemoModeUIUtils.cpp:39-56` — `WrapWithComingSoonOverlay(content, bShowOverlay, …)`: when `bShowOverlay` false, returns content unchanged. This is the single overlay seam used by all consumers.
- `Config\DefaultDemoMode.ini:1-29` — `bForceDemoMode=true`; AllowedHeroIDs Hero_1..5; AllowedDifficultyIDs=Easy; `bAllowLabRun=false`; `MaxDiplomaUpgradesPerStat=1`; `bAllowDrugPurchases=false`; `UnavailableContentText=COMING SOON`.

### Hero carousel coming-soon entries
- `Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp:200-203` builds `AllHeroIDs = GetAllHeroIDs()` (all heroes); `:420-450` each slot uses `bHeroPlayable = IsHeroPlayable(SlotHeroID)` and wraps non-playable slots in `WrapWithComingSoonOverlay(...)` (`:445-449`). **Hide approach:** filter `AllHeroIDs` to playable (e.g., via `FilterHeroIDs`) so non-owned slots are never created.

### Difficulty list coming-soon entries
- Same file `:219-229` builds `DifficultyOptions` from `GetVisibleDifficulties()` (all five). Dropdown `:744-777` marks non-playable as disabled + `bShowUnavailableOverlay` + `UnavailableText` (`:757-758`). **Hide approach:** source the list from `GetPlayableDifficulties()` (Easy-only in demo) so non-Easy never appears.
- Also consumes `GetVisibleDifficulties()`: `T66HeroSelectionScreen_Party.cpp:27,121`; `T66CompanionSelectionScreen.cpp:501,999,1033`. These must be reviewed for consistency in the hide phase.

### Daily Descent / Lab button source
- **Lab:** `T66HeroSelectionScreen_Build.cpp:638-660` — `bLabPlayable = IsRunCategoryPlayable(Lab)`; wraps in overlay when `!bLabPlayable` (`:646-660`). **Hide:** skip the canvas slot when not playable.
- **Daily Descent:** `T66MainMenuScreen.cpp:573-578` `IsDailyDescentAvailable()` = not demo mode; `:1200-1218` wraps in overlay when unavailable; `:2044-2054` `OnDailyDescentClicked` guards navigation. **Hide:** skip the CTA slot when unavailable (keep the click guard).

### Achievements gating (move to available)
- `T66AchievementsScreen.cpp:1096` `bDemoAchievementRowsLocked = IsDemoModeActive(this)`; Steam rows overlaid at `:1379-1391`; Secret rows overlaid at `:1759-1771`. **No config flag** — gated purely on demo mode. **Move to available:** drop `bDemoAchievementRowsLocked` (or force false) so both tabs render normally.

### Diploma upgrade gating (move to available)
- `T66PowerUpScreen.cpp:1951-1973` (full layout) and `:2426`, `:2579` (additional diploma overlay variants) gate on `Buffs->IsDemoDiplomaUpgradeLimitReached(StatType)` → `T66BuffSubsystem.cpp:844-855` → `!ReleaseVariant->IsDiplomaUpgradeAllowed(steps)` → `T66ReleaseVariantSubsystem.cpp:345-355` compares against `MaxDiplomaUpgradesPerStat` (config=1). **Move to available:** raise `MaxDiplomaUpgradesPerStat` to the full-game step count (`UT66BuffSubsystem::MaxFillStepsPerStat`; exact value to confirm in Phase 1) — config-only, easiest re-gate.

### Drugs gating (move to available) — **notable finding**
- `T66HeroSelectionScreen_Build.cpp:505-524` and `T66PowerUpScreen.cpp:1349`, `:2778` gate the Buy button on `Buffs->AreSingleUseBuffPurchasesAllowed()`.
- `T66BuffSubsystem.cpp:1055-1058` — `AreSingleUseBuffPurchasesAllowed()` **hard-returns `false`**, ignoring `bAllowDrugPurchases` and demo state entirely. Drugs are therefore *globally disabled*, not merely demo-gated. **Moving drugs to available is a code change**, not a config flip: e.g., delegate to `ReleaseVariant->AreDrugPurchasesAllowed()` and set `bAllowDrugPurchases=true`. Risk: the single-use-buff *purchase backend* may be a stub (see `:1060-1064`); enabling Buy could expose unimplemented purchase flow (Open Decision D1).

### Existing inventories
- None found (`**/{DEMO_GATED,DEPRECATED,…}*.md` → no files). Both `.md` files are net-new.

### Existing deprecated concept (for the deprecated doc)
- `Source\T66\Core\T66DeprecatedFeatureSettings.h/.cpp` + `Config\DefaultGame.ini:64-67`: `bDisableArcadeGames=true`, `bDisableArcadeInteractables=true`, `bDisableMinigames=true`. These are the authoritative *deprecated* (vs demo-gated) features — but they are Mini/arcade scope (Open Decision D2).

## 6. PPF And Process Gates

- **Process-governed? Largely no.** This is runtime UI visibility logic + config + docs, not reference-image UI fidelity, VFX, media, animation, audio, or generated content. The **UI reference-fidelity** process (and its fidelity loop) does **not** apply — no reference image is being matched; we are removing/hiding existing widgets.
- **No PPF/artifact-parity/mechanism manifest required** (no solved-category visual replication).
- **Applicable accepted process:** **Staged standalone verification** — these changes affect the playable standalone build, so the final phase must refresh the staged build and verify the shortcut target.
- **Proof-bearing:** Yes (compile logs + Unreal-owned UI captures proving overlays gone / entries hidden / achievements+diplomas+drugs available). Therefore mutating phases route to **FullOperator** after Codex approval; this read-only packet only plans.
- **No DataTable/data-asset import** in scope (changes are `.ini` config + C++), so the import/reload process is not triggered.

## 7. Proposed Patch Approach

### Phase plan (ordered to match user sequence)

**Phase 1 — Move-to-available (drugs, diplomas, achievements)** *(FullOperator, proof-bearing)* — detailed below.
**Phase 2 — Author two inventory `.md` docs** *(FullOperator; docs-only, no proof beyond doc review)*.
**Phase 3 — Hide demo-gated UI entries** (hero carousel, difficulty, Lab, Daily Descent; optionally companions) **+ update `DEMO_RELEASE_INSTRUCTIONS.md:100-101` rule** *(FullOperator, proof-bearing)*.
**Phase 4 — Staged standalone refresh + shortcut verification + final captures** *(FullOperator, proof-bearing)*.

Each phase needs its own `codex_operator_approval.md` before its FullOperator run; approving this plan does not pre-approve later phases.

### First mutating phase (Phase 1) — concrete approach

| Path | Planned change | Reason | Blast radius | Rollback |
|---|---|---|---|---|
| `Config\DefaultDemoMode.ini` | `bAllowDrugPurchases=true`; `MaxDiplomaUpgradesPerStat=<full step count>` (confirm `MaxFillStepsPerStat`) | Move drugs + diplomas into available via central gate | Demo-mode economy gating only; full-game unaffected | Revert two values |
| `Source\T66\Core\T66BuffSubsystem.cpp:1055-1058` | `AreSingleUseBuffPurchasesAllowed()` returns `ReleaseVariant->AreDrugPurchasesAllowed()` instead of hard `false` | Drug Buy is globally hard-disabled; config alone can't enable it | Drug Buy button enabled wherever this is consumed (HeroSelection + PowerUp); purchase flow `:1060-1064` now reachable | Restore `return false;` |
| `Source\T66\UI\Screens\T66AchievementsScreen.cpp:1096` | Set `bDemoAchievementRowsLocked = false` (remove demo gate; leave `:1379-1391`,`:1759-1771` blocks effectively no-op) | Achievements (Steam + secret) become available in demo | Achievements screen both tabs render live rows in demo | Restore `IsDemoModeActive(this)` |

**Phase 1 does not touch hero/difficulty/Lab/Daily Descent hiding** (that is Phase 3) and does not yet write docs (Phase 2).

## 8. Verification Plan

**Phase 1 (FullOperator):**
- Compile T66 editor target → expect success (pass marker: build "Build succeeded").
- Launch demo-mode frontend (`-T66Demo`) and capture via Unreal-owned UI capture (`CaptureT66UIScreen.ps1` / `-T66AutoDumpWidget`):
  - PowerUp → Diplomas: Graduate buttons enabled past step 1; no COMING SOON overlay (pass: no `*.DemoOverlay` surface present, buttons clickable).
  - HeroSelection + PowerUp → Drugs: Buy button enabled (pass: `DrugsPanel.BuyButton` enabled, no overlay).
  - Achievements → Steam and Secret tabs: live rows, no `Row##.DemoOverlay` (pass: overlay surfaces absent).
- **Failure meaning:** if Buy enables but purchase path is a stub, runtime error/no-op → triggers Open Decision D1.
- **Staged standalone:** Phase 1 *does* affect playable standalone, but staging is deferred to **Phase 4** to avoid repeated expensive stage passes between phases. Justification noted here per repo rule; Phase 4 performs `StageStandaloneBuild.ps1` + shortcut target check (`…\T66\Binaries\Win64\T66.exe`) + smoke once all code/config phases land.

**Phase 3:** captures proving hidden hero slots (only owned heroes), Easy-only difficulty dropdown, no Lab/Daily Descent buttons, and that `DEMO_RELEASE_INSTRUCTIONS.md` rule text is updated.

**Intentionally skipped this (read-only) phase:** all compile/run/capture — read-only profile cannot build or capture. If current compile/run/capture verification is requested now, this profile cannot satisfy it; a FullOperator run is required.

## 9. Token Routing

```
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: Unavailable (read-only run; manifest not exposed in this surface)
OperatorRunDir: n/a
OperatorManifest: n/a
CodexApprovalPath: Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval.md (not yet created)
ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Verify (1) T66BuffSubsystem.cpp:1055-1058 hard-false claim, (2) AchievementsScreen.cpp:1096 sole demo gate, (3) DEMO_RELEASE_INSTRUCTIONS.md:100-101 conflict, (4) Phase-1 scope excludes hide-work, (5) drug purchase backend readiness for D1.
```

## 10. Operator Position And Open Decisions

**Recommendation:** Approve the phase plan and **Phase 1** as scoped. Phase 1 is config + three localized code edits with clear rollback. Architecture already supports "easy re-enable" (central config-driven allowed lists; full-game variant shows everything), so no new framework is needed.

**Known caveats**
- `DEMO_RELEASE_INSTRUCTIONS.md:100-101` directly contradicts the new concept; Phase 3 must update that rule or the docs will misdirect future agents.
- Drug Buy is *globally* hard-disabled, not demo-gated — Phase 1 re-enables a code path that may have an unfinished purchase backend.

**Open decisions for the user / Validator**

- **D1 (drugs backend readiness) — recommend user input:** `AreSingleUseBuffPurchasesAllowed()` is hard-`false` and the purchase flow (`T66BuffSubsystem.cpp:1060-1064`) may be a stub. Enabling Buy could expose an unimplemented purchase. Choose: (a) enable now and fix/verify purchase flow in Phase 1, or (b) enable UI only and log a `pending_issues` entry if backend incomplete. → **NEEDS_HUMAN_DECISION** if the purchase backend is not ready.
- **D2 (deprecated doc + Mini scope):** The deprecated content (arcade games, arcade interactables, minigames) is Mini/arcade scope, which AGENTS.md excludes by default. The user asked for a *full* deprecated list. Confirm: list these deprecated Mini/arcade features in the doc (documentation only, no code touch) — recommended yes — or exclude Mini scope entirely.
- **D3 (companions in hide phase):** Companion COMING SOON overlays exist (`T66CompanionGridScreen.cpp:223`, `T66CompanionSelectionScreen.cpp:699`, `T66HeroGridScreen.cpp:204`). User named heroes/difficulty/Lab/Daily Descent but not companions. Recommend including companions in Phase 3 hiding for visual consistency; confirm.
- **D4 (inventory doc paths):** Propose `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` and `Demo/DEPRECATED_CONTENT.md`. Confirm location (Demo folder vs repo root for the deprecated one).

**Next approved phase:** Pending Codex approval artifact, run **Phase 1 (move-to-available)** as a FullOperator pass — *after* D1 is resolved, since D1 can change Phase 1's drug scope.

---

This read-only packet plans only; no files were edited and no build/capture was run. If you want current compile/run/capture proof, that requires a Codex-approved FullOperator run — this read-only profile cannot produce it.

**Codex Token Spent:**
Unavailable

**Claude Tokens Spent:**
Unavailable
