Verified the load-bearing claims against the repo. The draft holds up well — portrait remap, casino descriptor counts, demo allow-lists, frontend descriptors, and the Idle screen-owned gameplay all check out. Issuing the review.

Result: OK

## Summary
Codex's content audit is source-grounded, schema-compliant, and scope-correct (all four minigame modules audited at depth, routed to `Audit/Pending/` per repo rules). Spot-checks confirm its highest-stakes claims: the hero portrait remap (`Hero_1`→`Hero_4` art, `DefaultDemoMode.ini` allow-lists, the 6 casino descriptors with 3 demo-allowed, the four frontend descriptors, and the Idle `StartPlayableRun`/`ViewMode=Gameplay` screen-owned loop). The draft correctly does **not** misread the trailing `MakeFrontendDescriptor` booleans as visibility gates (the one trap the independent answer warned about). This is internally fixable polish, not a user decision. Proceed.

## Suggested Answer Patch
- **CONTENTFIND-014 / CONTENT-HERO-001 (undersells scope):** The portrait remap is not isolated to `Hero_1`. Verified: `Hero_4` row points at `Hero_6` art, `Hero_5` row points at `Hero_1` art ([Heroes.csv:5](C:/UE/T66/Content/Data/Heroes.csv:5), [Heroes.csv:6](C:/UE/T66/Content/Data/Heroes.csv:6)). Reword from "for example `Hero_1` uses `Hero_4` portrait paths" to note the remap spans at least three demo-visible heroes (1→4, 4→6, 5→1), so Inventory should treat it as a systematic remap, not a single stale row.
- **§9 minigame cards vs. independent structural read (reconcile, don't change tag):** Add one clause to CONTENT-MINI-004/005 noting that Idle/Deck gameplay is *screen-owned inside the MainMenu screen file* (no separate `BattleScreen` exists, unlike Mini/TD). I confirmed this at [T66IdleMainMenuScreen.cpp:816](C:/UE/T66/Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp:816). This justifies the `PARTIAL` tag (architectural asymmetry) and preempts a reviewer concluding "no gameplay" from the missing BattleScreen file.

## Issues To Fix
- The "core goal" demo-gating mechanism is traced via two independent surfaces (UI-manager screen block + top-bar omission) but the draft never names the underlying `DemoGateKind`/`FrontendMinigameLocked` descriptor field or the `ResolveDemoGateID` path. The conclusions are still correct, but for the ACTIVE-vs-DEMO_GATED distinction the user called central, Codex should add one STATIC_TRACE line tying the minigame gating to the release-variant resolver rather than only to UI suppression, so the tag rests on the gate source, not just two UI symptoms.
- Minor: §1 cites `T66ReleaseVariantSubsystem.cpp:103` for the `bForceDemoMode` honor and `:140` for `IsDemoModeActive`; §6/§13 cite `:84`, `:145`, `:220`, `:236`, `:247`, `:272`, `:297` for filters. These weren't individually re-verified here — Codex should confirm each filter line still resolves before finalizing, since this file is the spine of every DEMO_GATED tag.

## Question For User
None. The task is self-contained, read-only, and decided by repo rules — no user approval or decision unblocks it.

## Evidence Or Verification Gaps
- Zero `RUNTIME_VERIFIED` evidence exists; the draft states this plainly and backfills a §12 verification backlog — acceptable for a descriptive pass, but every "player experiences X" sentence rides on READ/STATIC_TRACE and must not harden into runtime claims.
- `CONTENTFIND-010` (T66Mini 66 bosses, 5 referenced) is the strongest ORPHAN_SUSPECT finding but rests on "by CSV count" rather than an exhaustive trace of every stage/wave `BossID` plus any randomizer. Flag explicitly that a hidden selector would collapse this finding — the draft does hedge this, keep that hedge.
- `Pets.csv` absence + boss-fallback synthesis (`CONTENTFIND-013`) is traced to `GetPetData()` but the DataTable-asset existence check is deferred to Inventory; the FALLBACK_ROUTE tag is sound as a suspicion, not a confirmed orphan.

## Notes
Counts, routing, schema tokens, and cross-reference IDs are consistent across §5/§10/§14. No git/file mutations implied. The draft's decision to treat stale comments/docs as claims-to-check (idol traveler, Idle/Deck master docs) is exactly the discipline the prompt demanded and materially changed three tags correctly.
