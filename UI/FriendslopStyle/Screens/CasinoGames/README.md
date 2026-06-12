# Casino Gambler Games — Hellfire Reimagine (2026-06-10)

Scope: the 4 live casino widget games (CoinFlip, GuessCup, StickPick, FindJoker) rebuilt
from text-button placeholders into sprite-driven animated games. This is content-artwork +
animation work inside the gambler tab; the tab chrome itself (buttons/panels) still renders
through the FriendslopStyle global bridge.

- Process: AGENTS.md "Image generation" row (account-backed Codex CLI imagegen workers,
  style-anchored on approved `mainmenu_v7.png`) + in-repo Slate animation method class
  (`FT66AnimationSequence`/`FT66SlateAnimationRunner` pattern, per the loot wheel/crate).
- Worker records: `Saved/Codex/UI/UIReimagine/impl_workers/casino_w1_coin..casino_w4_card/`.
- Runtime assets: `RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/CasinoGames/`
  loaded via `FT66FriendslopStyle::GetCustomBrush` (size-exact Image plates; only
  `table_stage.png` is a 9-slice Box surface, margin 0.06).
- Code: `Source/T66/UI/Gambler/` (`T66GamblerGameStage.*` shared kit + the 4 widgets),
  host flow in `Source/T66/UI/T66CasinoGamblerTabWidget.cpp`.
- Stage geometry: fixed 980x360 play stage (fits the gambler tab at 1280x720); all element
  sizes are px constants in code (`T66GamblerStage` + per-game namespaces) — the geometry
  table lives in code because every element is code-positioned, not reference-measured.

## Behavior contract changes (host)
- BET no longer auto-resolves with a random pick; it arms the round and the player picks
  in the game area (cups/sticks/cards/heads-tails). The bet-bar button becomes RANDOM PICK
  while waiting, preserving an always-available resolution path (and automation).
- Round outcome presentation (status line, Casino.Win/Lose stinger, double-down controls,
  gold counter refresh) is deferred until the game widget's reveal animation lands
  (`SetRevealCompleteCallback` -> `PresentPendingRoundOutcome`, 6s fallback timer).
  Outcome LOGIC (RNG, payout, anti-cheat record, round state) stays synchronous.

## Coverage matrix

| Game | Element | Asset | Widget | Status |
|---|---|---|---|---|
| Shared | play stage table plate | table_stage.png (9-slice) | T66GamblerStage::MakeStage | DONE |
| Shared | result banner | (text, Lilita One 30) | MakeResultBanner | DONE |
| Shared | win glow | win_glow.png | per-game GlowBox | DONE |
| CoinFlip | coin heads/tails/edge | coin_heads/tails/edge.png | CoinImage brush swap | DONE |
| CoinFlip | ground shadow | shadow_soft.png | ShadowBox | DONE |
| CoinFlip | landing burst | ember_burst.png | BurstBox | DONE |
| CoinFlip | toss physics (squash/arc/spin/bounce) | (animation) | SpinSequence | DONE |
| GuessCup | 3 cups | cup.png | FCupVisual x3 | DONE |
| GuessCup | token | token.png | TokenBox | DONE |
| GuessCup | tease/drop/shuffle/lift sequences | (animation) | ActiveSequence | DONE |
| StickPick | cauldron | stick_holder.png | CauldronWidget | DONE |
| StickPick | 5 sticks (cap + stretch shaft) | stick_cap.png + stick_shaft.png | FStickVisual x5 | DONE |
| StickPick | draw/reveal/length-consistency | (animation + constraint shuffle) | ActiveSequence | DONE |
| FindJoker | 10 cards back/joker/blank | card_back/joker/blank.png | FCardVisual x10 | DONE |
| FindJoker | deal-in/hover/flip sequences | (animation) | ActiveSequence | DONE |
| All | audio markers | Casino.CoinToss/CoinLand/CupShuffle/CardDeal/CardFlip/StickDraw | timeline markers | DONE |

## Asset registry

| File | Px | Source worker | Use |
|---|---|---|---|
| coin_heads.png | 380x380 | casino_w1_coin | coin face (190px box) |
| coin_tails.png | 380x380 | casino_w1_coin | coin face |
| coin_edge.png | 380x380 (content 360x~49 centered) | casino_w1_coin | mid-spin edge frame |
| shadow_soft.png | 400x96 | casino_w1_coin | ground shadow (200x48 box) |
| cup.png | 320x360 | casino_w2_cup | shell cup (160x180 box) |
| token.png | 152x152 | casino_w2_cup | prize token (76px box) |
| table_stage.png | 1536x1024 full-bleed | casino_w2_cup | stage plate, Box margin 0.06 |
| stick_cap.png | 68x60 | casino_w3_stick (crop) | stick gold cap (34x30 box) |
| stick_shaft.png | 68x240 | casino_w3_stick (crop) | stick shaft, stretches to length |
| stick_holder.png | 880x280 | casino_w3_stick | cauldron (440x140 box) |
| card_back.png | 200x280 | casino_w4_card | card back (100x140 box) |
| card_joker.png | 200x280 | casino_w4_card | joker face |
| card_blank.png | 200x280 | casino_w4_card | losing face |
| win_glow.png | 640x640 | casino_w4_card | win celebration glow |
| ember_burst.png | 440x440 | casino_w4_card | coin landing burst |

## Verification
- Capture automation: `-T66GameplayAutoCapture=casinocoinflip|casinoguesscup|casinostickpick|casinofindjoker`
  (opens gambler-only casino overlay, late-arms after the deferred tab rebuild, bets, picks
  on a timer) + frame-sequence flags via `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode casino<game>`.
- 2026-06-10 evidence (mp4 + 100-120 frame sequences + contact sheets per game):
  `Saved/VideoCaptures/CasinoGames_20260610/<Game>/`. CoinFlip shows a WIN flow (glow +
  deferred banner/controls); the other three captured LOSE flows (two-stage reveals).
- Anti-cheat round proof in capture logs: `[T66Proof][CasinoGameRound] Game=CoinFlip Bet=50
  Payout=100 Win=1 ... Actions=CoinFlip.Heads`.
- Known follow-up: casino overlay tab switching destroys gambler round state (pre-existing,
  documented in `Source/T66/UI/pending_issues_UI.md`).
