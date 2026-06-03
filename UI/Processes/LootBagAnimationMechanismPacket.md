# Loot Bag Animation Mechanism Packet

**Created:** 2026-05-26  
**Target:** LootBag post-interaction UI reward animation  
**Archetype:** `ContainerOpenReveal`  
**Owning process:** `UI/Processes/LootUIAnimationAuthoringProcedure.md`  
**Mini scope:** Excluded

## Source Evidence

- Internal process: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`.
- User target: a loot bag opens, the item card comes out of the bag, then the real item card opens.
- No external video reference was used for this target. No transcript-dependent video source was treated as process evidence.
- Existing runtime convention: generated T66-owned reward art may be used as equivalent primary UI art when approved, as in `UI/Processes/LootChestAnimationMechanismPacket.md`.
- Generated primary art:
  - `SourceAssets/UI/LootBagRewards/loot_bag_closed.png`
  - `SourceAssets/UI/LootBagRewards/loot_bag_open.png`
  - `RuntimeDependencies/T66/UI/LootBagRewards/loot_bag_closed.png`
  - `RuntimeDependencies/T66/UI/LootBagRewards/loot_bag_open.png`

Observed:

- Loot bag interaction already locks/grants the item before presentation in `Source/T66/Gameplay/T66PlayerController_Combat.cpp`.
- The normal inventory-space branch and Gambler's Token branch both now route through `ShowLootBagItemReveal`.
- `Source/T66/Gameplay/T66LootBagPickup.cpp` has `PrimaryActorTick.bCanEverTick = false` and no loot-bag tick/timeline wiggle path in the live actor.
- Existing pickup card dimensions, item icon binding, and item text are owned by the gameplay HUD pickup-card presentation.

Inferred:

- The LootBag-specific reveal should be a HUD-owned presentation lane that hands off to the existing item card instead of replacing global item-card behavior.
- The proxy card should use the same live item data as the final pickup card so the emergence frame cannot lie about the reward.

Tuned:

- Reveal lane height: `420`.
- Reveal duration: `1.42s`.
- Fade-out duration: `0.18s`.
- Sparkle count: `14`.
- Closed bag display size: `260x246`.
- Open bag display size: `286x270`.

## PPF CHECK

Objective: Build a LootBag UI animation where a sealed bag opens, the awarded item card rises out of it, and the real pickup item card takes over after the reveal.

Proven process: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`, with user-approved generated T66-owned closed/open bag assets as equivalent primary artifacts.

My planned implementation: Add a bag-specific HUD presentation entrypoint, route only LootBag item grants through it, load generated closed/open bag PNGs from `RuntimeDependencies`, animate closed-to-open bag states, raise a live item-data proxy card from the bag, then hand off to the existing pickup card. Leave crate, wheel, chest, and direct/debug item-card callers on their existing presentation paths.

Same method class: YES.

If NO, why: Not applicable.

User approval required before proceeding: Received in chat on 2026-05-26 after Claude review artifact `Saved/AgentReviews/20260526T090940-pass5/claude_review_pass5.md`.

Verification evidence: focused build, Unreal-owned lootbag capture, ffprobe, full and cropped contact sheets, runtime log markers, staged standalone refresh, staged asset proof, and shortcut target proof.

## ARTIFACT PARITY GATE

Reference artifact/category: closed loot bag state.  
Role: Primary.  
Required: YES.  
Planned artifact/path: `RuntimeDependencies/T66/UI/LootBagRewards/loot_bag_closed.png`, mirrored at `SourceAssets/UI/LootBagRewards/loot_bag_closed.png`.  
Status: EQUIVALENT, approved.  
Evidence: Original T66-owned generated sealed-bag art, not copied from external source art.

Reference artifact/category: open loot bag state.  
Role: Primary.  
Required: YES.  
Planned artifact/path: `RuntimeDependencies/T66/UI/LootBagRewards/loot_bag_open.png`, mirrored at `SourceAssets/UI/LootBagRewards/loot_bag_open.png`.  
Status: EQUIVALENT, approved.  
Evidence: Original T66-owned generated open-bag art with warm glow.

Reference artifact/category: item emerging from bag.  
Role: Primary.  
Required: YES.  
Planned artifact/path: Slate-owned `LootBagRevealCardBox` populated from the real item data and icon brush.  
Status: SAME.  
Evidence: The proxy card is populated by the same item data helper used for the final pickup card, then animated upward from the bag.

Reference artifact/category: real final item card.  
Role: Primary.  
Required: YES.  
Planned artifact/path: Existing `PickupCardBox` handoff through `CompleteLootBagRevealToPickupCard`.  
Status: SAME.  
Evidence: The reveal completes into the existing pickup-card surface instead of replacing the reward with a fake card.

Reference artifact/category: glow/sparkles.  
Role: Secondary.  
Required: NO.  
Planned artifact/path: Slate-owned sparkle widgets inside the LootBag reveal overlay.  
Status: EQUIVALENT.  
Evidence: Support layer only; not the primary carrier.

## MECHANISM MANIFEST

Reference/source: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`.

1. Mechanism: closed container state  
   Required: YES  
   Planned implementation: generated sealed bag sprite visible at the start of the presentation.  
   Evidence needed: capture frames before the open state appears.

2. Mechanism: opening motion or frame sequence  
   Required: YES  
   Planned implementation: pulse the closed bag, swap/fade into the generated open bag with glow and scale emphasis.  
   Evidence needed: contact sheet frames showing sealed bag, transition, and open bag.

3. Mechanism: result emergence or result handoff  
   Required: YES  
   Planned implementation: populate a proxy item card with the real item data and animate it upward from the open bag before handing off to `PickupCardBox`.  
   Evidence needed: multi-frame proof of proxy card emergence and a final frame with the real pickup card.

4. Mechanism: reveal timing  
   Required: YES  
   Planned implementation: keep the bag reveal visible for a tuned `1.42s` beat before a short fade and handoff.  
   Evidence needed: ffprobe duration/frame count plus contact sheet covering the reveal sequence.

5. Mechanism: commit/handoff marker  
   Required: YES  
   Planned implementation: grant the item through the existing run-state branch before showing the reveal, then log and hand off to the pickup card through `CompleteLootBagRevealToPickupCard`.  
   Evidence needed: runtime log markers and code route evidence.

6. Mechanism: skip/reset gating  
   Required: YES  
   Planned implementation: integrate the new lane into pending-presentation checks, queued pickup cards, `TrySkipActivePresentation`, hide/reset, and stale-widget cleanup.  
   Evidence needed: focused build and code route evidence.

7. Mechanism: no actor-model wiggle substitution  
   Required: YES  
   Planned implementation: keep the animation in the post-interaction HUD, not the world loot-bag actor.  
   Evidence needed: live actor has no tick/timeline wiggle path, and the capture route calls the HUD reveal directly.

## Anti-Lookalike

Cheapest wrong result: the bag interaction consumes the pickup and immediately shows the normal item pickup card, possibly with a static bag icon nearby.

Discriminator: accepted proof must show sealed bag -> open bag -> item proxy card emerging from the bag -> real pickup card handoff. A single static card or minimap loot-bag icon cannot pass.

## Implementation Notes

- Public HUD entrypoint: `UT66GameplayHUDWidget::ShowLootBagItemReveal`.
- Controller wrapper and capture route: `AT66PlayerController::ShowLootBagItemRevealHUD`.
- Runtime interaction route: both LootBag item branches in `T66PlayerController_Combat.cpp` call `ShowLootBagItemReveal`.
- Presentation owner: `Source/T66/UI/HUD/T66HUDPresentationController.cpp`.
- Slate surface owner: `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp`.
- Runtime path owner: `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h`.
- Generated art path: approved built-in account-backed `imagegen`; no `OPENAI_API_KEY` script path was used.

## PPF CLOSE

Process used: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`, with generated T66-owned sealed/open bag assets, proxy card emergence, and final item-card handoff.

Matches declared process: YES.

Evidence:

- Focused build succeeded with `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoUBTMakefiles`.
- Staged standalone refresh succeeded with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`.
- Standalone shortcuts `C:\UE\T66\T66 Standalone.lnk` and `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged runtime art exists under `Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI\LootBagRewards\`.
- Full Unreal-owned capture: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal.mp4` (`1280x720`, `12 fps`, `8.0s`, `96` frames).
- Trimmed review video: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal_trimmed.mp4` (`1280x720`, `12 fps`, `2.666667s`, `32` frames).
- Full mechanism contact sheet: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal_mechanism_sheet.png`.
- Cropped mechanism contact sheet: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal_mechanism_sheet_cropped.png`.
- Cropped trimmed-clip contact sheet: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal_trimmed_contactsheet_cropped.png`.
- Runtime log markers: `[LootBagReveal] started item=Item_AoeDamage rarity=2 via ShowLootBagItemReveal.`, `[LootUICapture] lootbag triggered via RunState AddItemWithRarity + ShowLootBagItemRevealHUD item=Item_AoeDamage rarity=Yellow.`, and `[LootBagReveal] handoff complete to pickup card.`

## MECHANISM CLOSE

Mechanism: closed container state  
Status: PRESENT  
Evidence: first captured frames show the sealed generated loot bag before the open bag appears.  
Discriminator test: not a direct item card; the sealed pre-open state is temporally visible.  
Reported status: FULL

Mechanism: opening motion or frame sequence  
Status: PRESENT  
Evidence: the cropped trimmed-clip contact sheet shows sealed bag, glow/open transition, and open bag across multiple frames.  
Discriminator test: not a static bag image beside a card; the visual state changes over time.  
Reported status: FULL

Mechanism: result emergence or result handoff  
Status: PRESENT  
Evidence: the cropped trimmed-clip contact sheet and MP4 show the proxy item card rising from the open bag, followed by the real pickup card with the same item.  
Discriminator test: not a fake static card; the proxy card emerges and then hands off to the real `PickupCardBox`.  
Reported status: FULL

Mechanism: reveal timing  
Status: PRESENT  
Evidence: the trimmed proof clip has 32 frames over 2.666667 seconds at 12 fps, covering the reveal and final handoff.  
Discriminator test: not a still image used as proof; temporal behavior is shown across a frame range.  
Reported status: FULL

Mechanism: commit/handoff marker  
Status: PRESENT  
Evidence: runtime log confirms the `ShowLootBagItemRevealHUD` route and the handoff marker; the combat branches grant first, then call `ShowLootBagItemReveal`.  
Discriminator test: not a capture-only overlay that bypasses the grant path.  
Reported status: FULL

Mechanism: skip/reset gating  
Status: PRESENT  
Evidence: the presentation controller includes LootBag reveal in active/pending checks, queued pickup routing, skip handling, hide/reset, and widget reset. Focused build and staged build both succeeded.  
Discriminator test: not a one-shot overlay that can leave stale widgets or block queued presentations.  
Reported status: FULL

Mechanism: no actor-model wiggle substitution  
Status: PRESENT  
Evidence: `AT66LootBagPickup` has actor ticking disabled and no tick/timeline wiggle path; the capture route calls the HUD reveal directly after adding the item.  
Discriminator test: post-interaction proof comes from the UI reveal, not from the world model moving.  
Reported status: FULL

## LOOT UI ANIMATION CLOSE

Target: LootBag  
Archetype: `ContainerOpenReveal`  
Process used: `UI/Processes/LootUIAnimationAuthoringProcedure.md`  
Artifact parity: FULL; all required primary artifacts are present, generated, and staged.  
Mechanism close: FULL; every required mechanism is present with multi-frame evidence.  
Anti-lookalike result: PASS; proof shows sealed bag, open bag, card emergence, and final real item-card handoff.  
Capture evidence: `Saved\VideoCaptures\LootBagReveal_20260526_1952\LootBag_reveal_trimmed.mp4` plus full MP4, ffprobe, runtime log, and contact sheets.  
Reported status: FULL
