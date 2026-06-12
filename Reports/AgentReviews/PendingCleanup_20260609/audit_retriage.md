# Audit/Pending Re-Triage Against Current Tree — 2026-06-09 (night)

Scope: the four audit documents in `Audit/Pending/` re-checked finding-by-finding against the
live tree as of tonight (post v1.2 `c8da91343` + pending-cleanup Phase 2 fixes, with the
parallel retro/archive cleanup agent mid-flight). Method: read-only grep/glob/read verification;
no audit document was edited. Classifications:

- **RESOLVED** — closed by a tree change or tonight's fix (evidence named).
- **STILL-OPEN-REAL** — genuine remaining defect; minimal fix stated in the defects list.
- **DEFERRED-BY-DESIGN** — intentional shelving / explicit out-of-scope decision.
- **STALE** — the audited thing no longer exists or the claim no longer applies.

Context that closed many findings wholesale: v1.2 deleted the entire arcade surface
(`T66Arcade*` gameplay/UI source, `ArcadeInteractables.json`/`DT_ArcadeInteractables`,
`Content/World/Interactables/Arcade*`, `Content/Audio/Arcade`, `SourceAssets/Arcade`,
`RuntimeDependencies/T66/Arcade`, `T66DeprecatedFeatureSettings.cpp`), the minigame
compatibility layer (`ET66ScreenType` has no Mini/TD/Idle/Deck/Versus tokens; `T66MinigamesScreen`
and `T66VersusArcadeScreen` deleted; `Gameplay/Minigames/` docs deleted), and the RetroFX
settings/subsystem source. `FT66ShelvedFeatureGate` was rewritten around four features:
DailyDescent, VehicleInteractables, Pets, MobLoot (all false). `Content/Data/Pets.csv` +
`DT_Pets.uasset` now exist (3 rows) and are wired in `T66GameInstance.cpp:352`.

---

## Per-document verdicts

| Document | Verdict | Why (one sentence) |
|---|---|---|
| `TechnicalAudit_Thinned_2026-06-02/TECHNICAL_AUDIT_THINNED.md` | **Move to Reference** | 19 of 25 findings are now resolved/stale/deferred and the architecture it maps (shelved minigame routes, arcade residue, deprecated-settings dual gate) no longer matches the tree; the 6 surviving real items are extracted into the defects list below. |
| `InventoryAudit_Thinned_2026-06-02/T66_Inventory_Audit_Thinned_2026-06-02.md` | **Move to Reference** | Its file/size census and arcade/minigame residue register predate the v1.2 purge (all counted residues are gone), and its 8 surviving items are all small hygiene/doc tasks captured below. |
| `T66_PERFORMANCE_SOURCE_AUDIT_2026-05-14.md` | **Move to Reference** | It is by its own framing a lexical triage sweep ("use as triage, not automatic guilt") whose actionable successor is an Unreal Insights capture, and its 524-file appendix is stale (arcade/retro files deleted, `T66HouseNPCBase` renamed). |
| `T66_CONTENT_AUDIT_2026-06-02.md` | **Move to Reference** | It audits the pre-demolition tree (five runtime modules, six-game casino with RPS/BlackJack) which the same-dated thinned audits superseded by their own scope statements; its 3 surviving real items already live in the pending ledgers. |

---

## 1. TECHNICAL_AUDIT_THINNED (TFIND) — 25 findings

| Finding | Classification | Evidence (current tree) |
|---|---|---|
| TFIND-001 idol traveler comment corrected | RESOLVED | Already closed by the audit itself; traveler delivery still live (`Idols.csv` all `Delivery=Traveler`; traveler fire requests in `T66CombatComponent.cpp`). |
| TFIND-006 T66Buried expected shelf missing | STALE | Zero `T66Buried`/`Buried` hits across Source/Config/Content/UI/Gameplay; prompt-premise artifact, nothing in tree to act on. |
| TFIND-010 traveler damage authority split | STILL-OPEN-REAL | Dual authority unchanged: `T66OutgoingTravelerPoolSubsystem.cpp:409` suppresses fallback when delegate bound, fallback applies at `:951`; close with one runtime double-damage proof or collapse the fallback path. |
| TFIND-011 snapshot omits in-flight projectiles/travelers | DEFERRED-BY-DESIGN | `T66RunStateSubsystem_Snapshot.cpp` (391 lines) still has zero projectile/traveler serialization — transient combat state intentionally not persisted (checkpoint-style save). |
| TFIND-012 pets boss-row fallback, no DT_Pets | DEFERRED-BY-DESIGN | Claim half-stale: `Content/Data/Pets.csv` + `DT_Pets.uasset` now exist (3 rows) and load via `T66GameInstance.cpp:352`; boss fallback retained for uncovered bosses and the pets feature is shelved (`bT66PetsEnabled=false`, gate line 9) — open by design per user. |
| TFIND-013 minigame docs/manifests residue | RESOLVED | `Gameplay/Minigames/` deleted entirely; `Gameplay/README.md` area list no longer names Mini/TD/Idle/Deck. |
| TFIND-014 power-up purchase silent failure | RESOLVED | Handlers now play `UI.Deny` on failed unlock/purchase and `UI.PowerUp.Confirm` on success (`T66PowerUpScreen.cpp:1509-1553`); the cited hero-selection unlock handler no longer exists. |
| TFIND-015 RetroFX cancel commits pending settings | STALE | `T66SettingsScreen_RetroFX.cpp` and all `*RetroFX*` source deleted (retro purge; remaining pixelation cluster owned by the parallel cleanup agent). |
| TFIND-016 Safe Mode / bug-report silent handlers | STILL-OPEN-REAL | `HandleSafeModeClicked` still applies settings with no visible/audible confirmation (`T66SettingsScreen_Crashing.cpp:51-58`); ReportBug live routes were retired and the screen is dead code tracked in `pending_issues_UI.md` archive-cleanup item. |
| TFIND-017 minigame routes retained as shells | STALE | Routes no longer retained: `T66UITypes.h` enum has only `DailyDescent=40`; frontend resolver and DirectEntry have zero Mini/Versus/Deck/Idle references. |
| TFIND-018 Versus screen shell remains | STALE | `T66VersusArcadeScreen.*` deleted; `VersusArcade` feature removed from the rewritten gate. |
| TFIND-019 Daily Descent shelved not deleted | DEFERRED-BY-DESIGN | Gate still hardcodes `bT66DailyDescentEnabled=false` (`T66ShelvedFeatureGate.cpp:7`); tonight added explanatory deny warnings (`T66DirectEntry.cpp:305`). |
| TFIND-020 arcade data preloads while shelved | RESOLVED | Zero arcade references left in `T66GameInstance.cpp`; `T66WidgetGameArcadeHelpers.cpp`, `ArcadeInteractables.json`, `DT_ArcadeInteractables` and all arcade dirs deleted. |
| TFIND-021 Item_VendorToken placeholder Backrooms icon | RESOLVED | `Items.csv:31` now references dedicated `/Game/Items/Sprites/Item_VendorToken_{black,red,yellow,white}` sprites. |
| TFIND-022 `gambler_results` legacy naming | DEFERRED-BY-DESIGN | Compatibility naming retained intentionally (`T66BackendRunSerializer.cpp:693-694`), mapping to the current four casino games. |
| TFIND-023 AppID 480 residue + backend doc drift | RESOLVED | `BACKEND_SYSTEM_REFERENCE.md:10` now carries an explicit status note framing 480 as historical-transition content; diagnostics 480 allowance is the documented transition window; the one remaining 480 is the stale local `WindowsHotfix` staged root (rolled into TFIND-028). |
| TFIND-024 anti-cheat doc stale on integrity_context | STILL-OPEN-REAL | `Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md:165,:262` still call `integrity_context` "future" while `T66RunIntegritySubsystem.cpp` + serializer + backend schema implement it; two-line doc fix. |
| TFIND-025 co-op per-member ticket proof absent | STILL-OPEN-REAL | `C:\UE\Backend\src\lib\schemas.ts` still contains zero ticket fields; submit-run auth remains a single host ticket — needs a recorded design acceptance (host attestation) or per-member ticket validation. |
| TFIND-026 client-config KV unverified, startup gate off | DEFERRED-BY-DESIGN | Startup explicitly logs "Frontend update gate disabled; continuing startup without client-config validation" (`T66PlayerController_Frontend.cpp:1310`) — deliberate disable; flag as release-checklist item. |
| TFIND-027 video catalog drift | STILL-OPEN-REAL | Narrowed but real: runtime and source manifests now share one structure (12 heroes/16 companions/4 fallbacks), but source carries 16 `Beachgoer` companion variants absent from runtime (34 leaf diffs) and README still claims 48/48/1 vs runtime 17 hero / 16 companion clips. |
| TFIND-028 staged-root ambiguity | STILL-OPEN-REAL | `Saved/StagedBuilds/{Windows,WindowsHotfix,WindowsTemp}` + `StagedBuildsDemo` + `StagedBuilds_PetMobLootFoundation` all persist; `WindowsHotfix/.../steam_appid.txt` still reads `480` — delete stale local roots (gitignored, zero repo risk). |
| TFIND-029 arcade selector PNG name mismatch | STALE | `T66ArcadeSelectionWidget.cpp` deleted with the arcade purge. |
| TFIND-030 localization residue (BlackJack/RPS) | RESOLVED | Zero `BlackJack`/`RockPaperScissors` matches in `Content/Localization/T66/T66.manifest`. |
| TFIND-031 deprecated secondary-stat enum entries | RESOLVED | Zero `DEPRECATED` enum entries remain in `T66DataTypes.h` after the v1.2 stats rework. |
| TFIND-032 removed item sprites shelved pending audit | RESOLVED | Tonight: Mini-inclusive reference audit found zero refs; 8 `Item_HpRegen_*`/`Item_LifeSteal_*` uassets deleted, `ImportItemSprites.py` preserve-list updated (`pending_issues_Data.md` resolution; zero files on disk). |

**Counts:** RESOLVED 9 · STILL-OPEN-REAL 6 · DEFERRED-BY-DESIGN 5 · STALE 5.

---

## 2. INVENTORY_AUDIT_THINNED (INVFIND) — 20 findings

| Finding | Classification | Evidence (current tree) |
|---|---|---|
| INVFIND-001 Mini/TD/Idle/Deck roots absent | RESOLVED | Confirmation finding; roots remain absent and the surviving route/shell surfaces it pointed at are now deleted too. |
| INVFIND-002 T66Buried expected shell | STALE | Zero hits tree-wide (same as TFIND-006). |
| INVFIND-003 per-mode shell docs stale | STALE | `Gameplay/Minigames/` (all four MasterImplementation docs + README) deleted. |
| INVFIND-004 UT66MinigamesScreen launcher residue | STALE | `T66MinigamesScreen.cpp` deleted. |
| INVFIND-005 UT66VersusArcadeScreen residue | STALE | `T66VersusArcadeScreen.cpp` deleted. |
| INVFIND-006 arcade content/source/runtime residues | RESOLVED | Every listed residue verified gone: `Content/World/Interactables/Arcade*`, `Content/Audio/Arcade`, `SourceAssets/Arcade`, `RuntimeDependencies/T66/Arcade`, `ArcadeInteractables.json`, `DT_ArcadeInteractables.uasset`, plus `T66DeprecatedFeatureSettings.cpp`. |
| INVFIND-007 pets table seam | DEFERRED-BY-DESIGN | `Pets.csv`/`DT_Pets` now exist (3 rows) and are wired; boss fallback retained; pets shelved via gate — open by design. |
| INVFIND-008 HpRegen/LifeSteal sprite orphans | RESOLVED | 8 uassets deleted tonight after zero-reference audit; `Content/Items/Sprites` glob confirms none remain. |
| INVFIND-009 status effects assigned None | DEFERRED-BY-DESIGN | Ledger keeps the mob-status feature deliberately out of scope (decision block); separate DisplayName cell corruption in `StatusEffects.csv` was fixed tonight. |
| INVFIND-010 video README/runtime drift | STILL-OPEN-REAL | `Video Generation/README.md` coverage line still claims 48/48/1 against a runtime catalog of 17 hero / 16 companion / 1 main-menu clips — one-line doc correction after catalog sync. |
| INVFIND-011 video source/runtime manifest mismatch | STILL-OPEN-REAL | Narrowed: structures now identical except 16 source-only `Beachgoer` companion entries missing from the runtime catalog (34 leaf diffs) — re-run the catalog sync. |
| INVFIND-012 WebView2 license/version manifest | STILL-OPEN-REAL | `ThirdParty/WebView2/` still contains only `bin`/`include`; no LICENSE or version manifest — drop in the license file and pin the SDK version. |
| INVFIND-013 content stubs registry missing | RESOLVED | `UI/content_stubs_registry.md` now exists, satisfying `UI_FIDELITY_LOOP_INSTRUCTIONS.md:968`. |
| INVFIND-014 empty placeholder roots | STILL-OPEN-REAL | All six remain empty (`Content/Collections`, `Content/Developers`, `Content/T66MapAssets`, `Content/VFX/Idols`, `Content/UI/Sprites/Interactables`, `Content/VFXLab/Temp/MRQ`) — trivial local deletion. |
| INVFIND-015 .pyc cache residue | STILL-OPEN-REAL | `Scripts/__pycache__` grew to 56 `.pyc` — gitignored; trivial deletion (or leave as ignored noise by policy). |
| INVFIND-016 temp scratch roots | STILL-OPEN-REAL | `tmp/` grew to 894 files and `UET66SavedTmpCombatVFXValidatorSelfTest_Bounce` persists — trivial local deletion. |
| INVFIND-017 PIXALTEST provenance | STILL-OPEN-REAL | Five `Content/Characters/Mobs/PIXALTEST*` dirs remain and `T66GameMode.cpp:980-989` still soft-refs them — decide keep-as-dev-harness vs delete refs+dirs. |
| INVFIND-018 performance schema drift | RESOLVED | `PerformanceSystem/README.md:64` now states runtime schema 8, matching `T66PerformanceSubsystem.cpp:44` (`T66PerformanceSchemaVersion = 8`). |
| INVFIND-019 Audit README pending drift | STILL-OPEN-REAL | `Audit/README.md:13` still says "No active pending audit files" while `Audit/Pending/` holds these audits — one-line fix already queued in Phase 4 of the cleanup program. |
| INVFIND-020 deprecated vs shelved gate coexistence | RESOLVED | `T66DeprecatedFeatureSettings.cpp` deleted; the rewritten `FT66ShelvedFeatureGate` is the single gate (DailyDescent/Vehicles/Pets/MobLoot). |

**Counts:** RESOLVED 6 · STILL-OPEN-REAL 8 · DEFERRED-BY-DESIGN 2 · STALE 4.

---

## 3. T66_PERFORMANCE_SOURCE_AUDIT_2026-05-14 — document-level classification

Not re-verified file-by-file (per task instruction). The document is a **descriptive lexical
flag sweep** — regex keyword presence over 524 translation units, explicitly framed as triage
("flags are lexical regex hits... not automatic guilt") with its own stated follow-up being an
Unreal Insights capture of tower combat / main menu / run summary. It carries no finding IDs and
no fix queue, so it was never a "pending fixes" document: **reclassify to `Audit/Reference/`**,
and treat a fresh Insights capture (post stats-rework, post lightweight-mob migration, post
retro purge) as its actionable successor — the B.10-B.12 lightweight-mob program has already
superseded much of its enemy-cost guidance.

Status of its 11 four-or-more-flag triage files in the current tree:

| File | Current state |
|---|---|
| `T66PlayerController.cpp`, `T66CombatComponent.cpp`, `GameMode/T66GameMode_BossFlow.cpp`, `T66VisualUtil.cpp`, `T66PlayerController_Combat.cpp`, `T66MiasmaManager.cpp`, `T66LavaPatch.cpp`, `T66HeroBase.cpp`, `T66GameMode.cpp` | Still exist (9 of 11). |
| `T66HouseNPCBase.cpp` | **Renamed** to `T66NPCBase.cpp` (`AT66NPCBase`, CoreRedirects preserved — pending_issues_Gameplay "NPC Class Names" resolution). |
| `T66ArcadeMachineInteractable.cpp` | **Deleted** (v1.2 arcade purge; no `*Arcade*` files under `Source/T66/Gameplay`). |

Also stale in its interpretation layer: `T66RetroFXSubsystem.cpp` (deleted in the retro purge)
and the arcade trio `T66ArcadeAmplifierPickup` / `T66ArcadeInteractableBase` /
`T66ArcadeTruckInteractable` (deleted), so its 524-file appendix overcounts the current module.

---

## 4. T66_CONTENT_AUDIT_2026-06-02 (CONTENTFIND) — 15 findings

This audit describes the **pre-demolition** tree (five runtime modules incl. T66Mini/TD/Idle/Deck,
six-game casino with RockPaperScissors/BlackJack/Lottery/Plinko/BoxOpening, arcade allow-lists).
The same-dated thinned audits superseded it by their own scope statements.

| Finding | Classification | Evidence (current tree) |
|---|---|---|
| CONTENTFIND-001 authored vs demo-visible confusion | STALE | Observation-class, not a defect; its counts no longer hold (demo companions now 3, casino allow-list is the four new games) — superseded by the thinned audits. |
| CONTENTFIND-002 minigames demo-gated not deprecated | STALE | No longer true in either direction: minigame modules, screens, routes and docs are deleted; the surviving model is the rewritten shelved gate. |
| CONTENTFIND-003 arcade allow-list vs deprecated gate | RESOLVED | `AllowedArcadeGameIDs` removed from `DefaultDemoMode.ini`; arcade surface purged; `T66DeprecatedFeatureSettings.cpp` deleted — the mismatch cannot recur. |
| CONTENTFIND-004 idol traveler comments stale | STILL-OPEN-REAL | The two stale comments persist at `T66DataTypes.h:1056` ("reserved for the Foundation API adapter") and `:1996` ("inert until... adapter lands") while traveler delivery is live — two-comment fix. |
| CONTENTFIND-005 status effects not production-assigned | DEFERRED-BY-DESIGN | `pending_issues_Data.md` keeps it open deliberately (validator requires `None`; mob status-effect feature explicitly out of scope in the decision block). |
| CONTENTFIND-006 spawn director fallback families | STILL-OPEN-REAL | `pending_issues_Gameplay.md` "Spawn Director Still Uses Fallback-Family Behavior" remains open [Major]; minimal close = the already-specified archetype-aware director refactor (Hell-no-ranged remains intentional). |
| CONTENTFIND-007 missing data refs (DT_HouseNPCs etc.) | RESOLVED | Tonight: `BP_T66GameInstance` repointed `DT_HouseNPCs`→`DT_NPCs` (binary verified) and dead `BrokenVase_Easy` row removed; `Item_Headshot` sprites fixed by v1.2; `Item_GamblersToken`/`Item_Alchemy` traced to stale-cook residue with closure riding the next staged cook log. |
| CONTENTFIND-008 tower floor / drop-hole seams | DEFERRED-BY-DESIGN | Both ledger items remain open with documented fix paths and "floor-seam rework" is explicitly out of scope in tonight's decision block. |
| CONTENTFIND-009 controller focus + loot-wheel toast | STILL-OPEN-REAL | Both `pending_issues_UI.md` items remain open; the focus contract is explicitly deferred (decision block), leaving the loot-wheel boost result toast as the unowned actionable gap. |
| CONTENTFIND-010 T66Mini 66-row boss table orphans | STALE | T66Mini module, content and data deleted. |
| CONTENTFIND-011 T66Idle stale docs / hero naming clash | STALE | T66Idle module and its docs deleted. |
| CONTENTFIND-012 T66Deck stale docs / prototype scale | STALE | T66Deck module and its docs deleted. |
| CONTENTFIND-013 pet boss-fallback route | DEFERRED-BY-DESIGN | `Pets.csv`/`DT_Pets` now authored (3 rows) with fallback retained; pets shelved via gate — open by design per user. |
| CONTENTFIND-014 hero/companion portrait remaps | RESOLVED | `Heroes.csv` 12/12 rows reference their own `Hero_N` sprite folders and `Companions.csv` 16/16 reference their own `Companion_NN` folders — no remaps remain. |
| CONTENTFIND-015 vendor failed-steal hidden boss route | DEFERRED-BY-DESIGN | Hidden content by design; `ResolveShopStealAttempt` still live (`T66RunStateSubsystem_EconomyInventory.cpp:616`) — remains a runtime-verification backlog item, not a defect. |

**Counts:** RESOLVED 3 · STILL-OPEN-REAL 3 · DEFERRED-BY-DESIGN 4 · STALE 5.

---

## Genuine remaining defects worth fixing (extracted from STILL-OPEN-REAL rows)

Ordered roughly by effort-to-value; items 1-3 are minutes of work.

1. **Two stale idol traveler comments** — `T66DataTypes.h:1056` and `:1996` still say traveler
   delivery is reserved/inert; it ships live. (CONTENTFIND-004)
2. **Anti-cheat policy doc lags implementation** — `Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md:165,:262`
   call `integrity_context` future/missing; client+backend implement it. (TFIND-024)
3. **`Audit/README.md:13` contradiction** — "No active pending audit files" while Pending holds
   these audits; already queued in Phase 4, this document is part of closing it. (INVFIND-019)
4. **Video catalog sync + README** — runtime catalog lacks the 16 `Beachgoer` companion clips
   present in the source manifest; README coverage line (48/48/1) does not match the runtime
   catalog (17/16/1). Re-run the manifest sync, then correct the README. (TFIND-027, INVFIND-010/011)
5. **Safe Mode apply has no confirmation** — `T66SettingsScreen_Crashing.cpp:51-58` applies
   settings silently; wire the existing `UI.PowerUp.Confirm`/`UI.Deny` audio pattern or a toast.
   (TFIND-016; the ReportBug half is dead code already tracked for archive cleanup)
6. **Spawn director fallback-family routing** — open [Major] ledger item; archetype-aware
   quota refactor specified in `pending_issues_Gameplay.md`. (CONTENTFIND-006)
7. **Loot-wheel boost result toast** — boost rewards commit with no focused presentation;
   queued-toast lane design already written in `pending_issues_UI.md`. (CONTENTFIND-009)
8. **Co-op per-member auth gap** — submit-run validates one host ticket; member summaries are
   host-attested with no per-member ticket field in `schemas.ts`. Either record the host-attestation
   trust decision in the anti-cheat doc or add per-member ticket validation. (TFIND-025)
9. **Outgoing-traveler dual damage authority** — mitigation present (fallback suppressed when
   callback bound) but unproven; one runtime double-damage test closes it, or collapse the
   fallback damage path entirely. (TFIND-010)
10. **Local hygiene batch (one deletion pass, all gitignored/local-only):** stale staged roots
    `Saved/StagedBuilds/WindowsHotfix` (still carries `steam_appid.txt`=480), `WindowsTemp`,
    `StagedBuildsDemo`(if superseded), `StagedBuilds_PetMobLootFoundation`; `tmp/` (894 files) and
    `UET66SavedTmpCombatVFXValidatorSelfTest_Bounce`; `Scripts/__pycache__` (56 .pyc); the six
    empty Content placeholder roots. (TFIND-028, INVFIND-014/015/016)
11. **WebView2 third-party compliance** — add the referenced LICENSE file and a version manifest
    under `ThirdParty/WebView2/`. (INVFIND-012)
12. **PIXALTEST residue decision** — five `Content/Characters/Mobs/PIXALTEST*` dirs plus live
    soft refs at `T66GameMode.cpp:980-989`; either bless as a permanent dev harness or remove
    refs and dirs together. (INVFIND-017)

---

## Summary counts

| Document | RESOLVED | STILL-OPEN-REAL | DEFERRED-BY-DESIGN | STALE | Total |
|---|---:|---:|---:|---:|---:|
| Technical (thinned) TFIND | 9 | 6 | 5 | 5 | 25 |
| Inventory (thinned) INVFIND | 6 | 8 | 2 | 4 | 20 |
| Content CONTENTFIND | 3 | 3 | 4 | 5 | 15 |
| Performance source audit | — | — | — | — | document-level: move to Reference; 2 of 11 high-flag files gone (1 deleted, 1 renamed) |
| **Total findings** | **18** | **17** | **11** | **14** | **60** |

Of the 17 STILL-OPEN-REAL rows, several share a root task, collapsing to the 12 defects above —
and of those, only items 5-9 touch shipping behavior; the rest are doc/data/hygiene.
