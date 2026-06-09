# Friendslop Migration — Morning Review Packet

Run: overnight 2026-06-09 → 2026-06-10. Branch: `friendslop-migration` (not pushed).
Charter: `decision_charter.md`. Full heartbeat: `overnight_log.md`.

## TL;DR

The goal — **FlatStyle legacy by morning** — is done, and done at the infrastructure level
rather than screen-by-screen. All ~450 FlatStyle call sites across every screen, overlay,
modal, and the HUD now render FriendslopStyle plates through one choke point
(`MakeFlatPanelSurface`) behind `T66.UI.FriendslopGlobal` (default on, `-T66FlatLegacy`
escape). The whole game flipped at once; the 25-surface capture sweep shows a coherent
dark-inflatable look with red/green accents everywhere, and zero flip-caused big issues.

## How it works

- `FT66FriendslopStyle` (pre-existing, from the MainMenu pilot) renders plates with cached
  brushes + graceful fallback. The flip routes FlatStyle's internals into it:
  - `MakeFlatPanelSurface` — universal: panels, buttons, toggle/icon/tab buttons,
    dropdowns, sliders, overlay panels/slots. Interactive surfaces get the state-mapped
    button plate (dark/red/green); static get PanelLargeDark. Hover = white film,
    disabled = dark film, so dynamic recolors by callers still work.
  - `MakeHudPanel` — Friendslop panel + preserved gold title row.
  - Legacy `MakeButton(FT66ButtonParams)` (in-run overlays) — translated to plate buttons.
- Already-migrated screens (MainMenu, AccountStatus tabs, tooltips, modals) untouched.
- Rollback: set `T66.UI.FriendslopGlobal 0` or launch with `-T66FlatLegacy`. One switch.

## Evidence

- Triage sheet (25 captures): `Saved/Codex/UI/FriendslopMigration/round1/triage_sheet.png`
- Standouts reviewed full-size: pause menu (excellent), vendor/casinoshop (good),
  idol altar (good), settings (good), load game (good), HUD (good).
- Wiring sanity: smoke suite PASS — all 7 frontend tag-click cases (including case 04,
  which was failing on the 2026-06-08 baseline) + DurableSaveIntegrity PASS;
  LifecycleTransition reports BUILD_CONFIG_UNSUPPORTED (config status, not a failure).
  First attempt hit one screenshot-timeout flake on case 05 (click+nav+dump all succeeded
  in its log); retry passed everything. Run: Saved/PreReleaseSmokeSuite/20260609_175117.

## Morning decisions for you (none block anything)

1. **Merge?** Review the triage sheet; if the direction holds, merge `friendslop-migration`
   to main. Every change is on the branch in 2 commits (flip + docs/packet).
2. **Cosmetic nits queue** (logged, deliberately not fixed per your guardrails):
   - Pause: RESUME label rides high on the red pill; bottom LEADERBOARD button clips panel.
   - Settings: very wide dropdown bar stretches its slice thin.
   - Loading screen: plain text, no plate.
3. **Pre-existing issues found (not from tonight):**
   - Gambler tab: magenta frame + empty content — identical with the flip off
     (`round1/casinogambling_LEGACY.png`). Was broken before tonight.
   - Lab + crate capture modes don't open their overlays (capture-route issue).
   - DailyDescent + PetSelection screen automation names don't resolve.
4. **Next-level polish path** (future nights): per-screen reference-driven passes under the
   FriendslopStyle authority doc for the surfaces you care most about; the flip is the
   baseline, not the ceiling. Also consider the Lilita One font for flat-path labels.

## Docs updated (FlatStyle officially legacy)

- `UI/UI_AGENTS.md` — FriendslopStyle is the active lane; FlatStyle = legacy adapter.
- `ART_DIRECTION.md` — UI boundary updated with the global flip.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — flip note + code pointer.
