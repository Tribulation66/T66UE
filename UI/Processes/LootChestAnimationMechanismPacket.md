# Loot Chest Animation Mechanism Packet

**Created:** 2026-05-26  
**Target:** LootChest post-interaction UI reward animation  
**Archetype:** `ContainerOpenReveal`  
**Owning process:** `UI/Processes/LootUIAnimationAuthoringProcedure.md`  
**Mini scope:** Excluded

## Source Evidence

- Concrete visual reference: Tenor Vampire Survivors loot chest GIF: <https://tenor.com/view/vampire-survivors-loot-chest-reward-gamble-gif-25270155>
- Local reference artifact: `Saved/Codex/UI/LootChest/Reference/vampire_survivors_loot_reference.gif`
- Local reference contact sheet: `Saved/Codex/UI/LootChest/Reference/vampire_survivors_loot_reference_contactsheet.png`
- Reference probe: 640x640, 20 fps, 13.75 seconds, 275 frames.

Observed source mechanisms:

- closed chest is visible before the reveal,
- lid opens upward into an open chest state,
- coins/items burst upward and outward from the chest,
- strong warm beams and sparkles sell the reveal,
- reward/currency is visible after the opening begins.

## PPF CHECK

Objective: Build a Vampire Survivors-style Loot Chest UI animation where the chest opens and many coins burst out before the gold reward finishes.

Proven process: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`, using the Tenor Vampire Survivors chest GIF as visual reference evidence.

My planned implementation: Use original generated T66-owned closed chest, open chest, and coin sprites; show the existing `StartChestRewardHUD` presentation as a centered chest-specific UI surface; animate closed to open, emit staggered coin arcs, add light beams/sparkles, preserve count-up, skip/reset, queueing, and `Chest.InventoryCommit`.

Same method class: YES, with user-approved equivalent primary generated assets.

If NO, why: Not applicable.

User approval required before proceeding: Received in chat on 2026-05-26 after Claude review artifact `Saved/AgentReviews/20260526T071613-pass2/claude_review_pass2.md`.

Verification evidence: focused build, Unreal-owned lootchest capture, ffprobe, contact sheet, visibility check, and staged standalone refresh.

## ARTIFACT PARITY GATE

Reference artifact/category: closed chest sprite.  
Role: Primary.  
Required: YES.  
Planned artifact/path: `RuntimeDependencies/T66/UI/ChestRewards/chest_reward_yellow_closed.png`, mirrored at `SourceAssets/UI/ChestRewards/chest_reward_yellow_closed.png`.  
Status: EQUIVALENT, approved.  
Evidence: Original T66-owned generated art, not copied reference art.

Reference artifact/category: open chest sprite.  
Role: Primary.  
Required: YES.  
Planned artifact/path: `RuntimeDependencies/T66/UI/ChestRewards/chest_reward_yellow_open.png`, mirrored at `SourceAssets/UI/ChestRewards/chest_reward_yellow_open.png`.  
Status: EQUIVALENT, approved.  
Evidence: Same generated chest identity with lid open and warm glow.

Reference artifact/category: coin burst/result emergence.  
Role: Primary.  
Required: YES.  
Planned artifact/path: `RuntimeDependencies/T66/UI/ChestRewards/chest_reward_coin.png`, mirrored at `SourceAssets/UI/ChestRewards/chest_reward_coin.png`.  
Status: EQUIVALENT, approved.  
Evidence: Original generated coin sprite emitted by the UI animation.

Reference artifact/category: beams and sparkles.  
Role: Secondary.  
Required: YES for this visual pass.  
Planned artifact/path: Slate-owned beam and sparkle widgets inside the chest reward surface.  
Status: EQUIVALENT.  
Evidence: Procedural UI beam/sparkle layers support the reference's celebratory chest reveal.

Reference artifact/category: multi-item fan/cards.  
Role: Secondary.  
Required: NO.  
Planned artifact/path: Deferred.  
Status: DEFERRED.  
Evidence: Current T66 LootChest reward is a gold reward; user requested chest plus coins for this pass.

## MECHANISM MANIFEST

Reference/source: `ContainerOpenReveal` procedure plus Vampire Survivors GIF/contact sheet.

1. Mechanism: closed container state  
   Required: YES  
   Planned implementation: generated closed chest sprite visible at the start of the presentation.  
   Evidence needed: capture frames before opening transition.

2. Mechanism: opening motion/frame sequence  
   Required: YES  
   Planned implementation: pulse the closed chest, fade/swap into generated open chest with overshoot.  
   Evidence needed: contact sheet frames showing closed, transition, and open states.

3. Mechanism: result emergence / coin burst  
   Required: YES  
   Planned implementation: staggered generated coin sprites fly upward/outward from the open chest mouth.  
   Evidence needed: multi-frame proof that coins originate near the chest and disperse.

4. Mechanism: reveal timing / count-up  
   Required: YES  
   Planned implementation: preserve `FT66AnimationTimeline` count-up from 0 to the locked gold amount.  
   Evidence needed: capture or runtime log showing deterministic `+188` route.

5. Mechanism: commit/handoff marker  
   Required: YES  
   Planned implementation: preserve `Chest.InventoryCommit` marker and callback dispatch.  
   Evidence needed: code path and successful capture completion.

6. Mechanism: skip/reset gating  
   Required: YES  
   Planned implementation: preserve `TrySkipActivePresentation`, `HideChestReward`, and queued reward handling.  
   Evidence needed: build/code review; no capture-only shortcut.

7. Mechanism: no world-model wiggle substitution  
   Required: YES  
   Planned implementation: only target the post-interaction HUD presentation.  
   Evidence needed: diff does not touch chest actor/model animation paths.

## Anti-Lookalike

Cheapest wrong result: a static chest reward card with a gold number and decorative coins.

Discriminator: accepted proof must show closed chest -> opening transition -> open chest -> multiple coins bursting upward/outward -> count/commit/fade. A still image cannot pass.

## Implementation Notes

- Live trigger: `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` deterministic `lootchest` capture route.
- Presentation owner: `Source/T66/UI/HUD/T66HUDPresentationController.cpp`.
- Slate surface owner: `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp`.
- Runtime path owner: `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h`.
- Image generation worker prompt: `Saved/Codex/UI/LootChest/loot_chest_imagegen_worker_prompt.md`.
- CLI Codex imagegen worker timed out before producing usable image files; final assets were generated through the approved built-in account-backed `imagegen` path using the same worker prompt/spec.

## PPF CLOSE

Process used: `UI/Processes/LootUIAnimationAuthoringProcedure.md` `ContainerOpenReveal`, with Vampire Survivors chest GIF reference and generated T66-owned closed/open chest plus coin sprite assets.

Matches declared process: YES.

Evidence:

- Focused build succeeded with `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`.
- Staged standalone refresh succeeded with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`.
- Standalone shortcuts `C:\UE\T66\T66 Standalone.lnk` and `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Runtime chest art exists in the staged build under `Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI\ChestRewards\`.
- Full Unreal-owned capture: `Saved\VideoCaptures\LootChestVampire_20260526_0808\LootChest_vampire_style.mp4`.
- Trimmed review video: `Saved\VideoCaptures\LootChestVampire_20260526_0808\LootChest_vampire_style_trimmed.mp4` (`1280x720`, `12 fps`, `2.0s`, `24` frames).
- Mechanism contact sheet: `Saved\VideoCaptures\LootChestVampire_20260526_0808\LootChest_vampire_style_mechanism_sheet.png`.
- Runtime log marker: `[LootUICapture] lootchest triggered via StartChestRewardHUD rarity=Yellow gold=188.`

## MECHANISM CLOSE

Mechanism: closed container state  
Status: PRESENT  
Evidence: first captured frames show generated closed chest with `+0` before the lid opens.  
Discriminator test: not a static open-result card; the closed pre-open state is temporally visible.  
Reported status: FULL

Mechanism: opening motion/frame sequence  
Status: PRESENT  
Evidence: contact sheet shows closed chest, glow/open transition, and final open chest with overshoot.  
Discriminator test: not an instant texture swap; the opening occurs across multiple frames.  
Reported status: FULL

Mechanism: result emergence / coin burst  
Status: PRESENT  
Evidence: contact sheet and MP4 show generated coin sprites emerging from the chest mouth and dispersing upward/outward.  
Discriminator test: not decorative static coins; the coins originate near the opened chest and travel over time.  
Reported status: FULL

Mechanism: reveal timing / count-up  
Status: PRESENT  
Evidence: deterministic capture starts at `+0` and resolves to `+188`; log confirms the `gold=188` route.  
Discriminator test: not a pre-filled reward label; the count begins after the opening beat.  
Reported status: FULL

Mechanism: commit/handoff marker  
Status: PRESENT  
Evidence: implementation preserves `Chest.InventoryCommit` callback dispatch and queued reward completion; deterministic capture completed without presentation hang.  
Discriminator test: not a capture-only overlay that bypasses the reward handoff path.  
Reported status: FULL

Mechanism: skip/reset gating  
Status: PRESENT  
Evidence: implementation preserves `TrySkipActivePresentation`, `HideChestReward`, and queue reset paths while resetting chest/coin/beam/sparkle UI state on hide.  
Discriminator test: presentation can be reset and reused instead of leaving stale animated child widgets.  
Reported status: FULL

Mechanism: no world-model wiggle substitution  
Status: PRESENT  
Evidence: animation changes are confined to the post-interaction HUD presentation and generated UI runtime assets; chest actor/model animation paths were not edited for this pass.  
Discriminator test: live interaction can go straight to UI presentation rather than depending on a world-model wiggle to sell the reward.  
Reported status: FULL
