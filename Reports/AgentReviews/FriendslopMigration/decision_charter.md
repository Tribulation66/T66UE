# Friendslop Full-UI Migration — Overnight Decision Charter

Date: 2026-06-09 (overnight run)
Authorized by: Pablo (explicit, this session)

## Mission

Migrate ALL player-facing UI (HUD, screens, overlays, modals, vendor, settings, etc.)
from FlatStyle to FriendslopStyle in one autonomous overnight run. By morning,
FlatStyle is legacy infrastructure. Perfection is NOT the goal — coverage and
infrastructure completion are. This is not the last UI pass.

## Explicit user authorizations (override default decision-gate stops)

- Run continuously without asking permission or posting status to chat.
  Status goes to `overnight_log.md` next to this file, updated after every step.
- Art direction: follow the FriendslopStyle process anchored on the existing
  approved Friendslop screens (Main Menu Round06 + minimap candy direction);
  Claude's taste fills the gaps. No per-screen reference approval tonight —
  user reviews everything in the morning.
- Git: branch `friendslop-migration`, one commit per completed screen/batch,
  NO push. (Explicit user approval for non-main branch.)
- Blanket APPROVED_REUSE: existing Main Menu Friendslop assets and tonight's
  kit assets may be reused across all screens.

## Standing decisions (so nothing needs asking at 4am)

- Reskin, not redesign: layouts, data bindings, handlers, navigation stay
  exactly as-is. Only chrome/material/style/icons change.
- Style conflicts → follow the kit. Layout questions → preserve current layout.
- Live text/data is NEVER baked into raster art.
- A screen that can't be made right safely → revert its changes, mark DEFERRED
  in the log, continue. Never leave a screen half-broken.
- Iteration caps: 1 pass per screen; 2 only for big issues (unreadable text,
  broken layout, missing chrome, crash). Cosmetic nits → log for morning.
- Same failure twice → revert + defer + continue. Cook flake → 2 retries.
- Codex worker failure → fresh worker, backoff; imagegen fully dead → continue
  code-side work, defer asset-dependent items, note in log.
- Global abort ONLY for unfixable repo-wide breakage → revert to last good
  commit, continue remaining safe work (docs, reports).

## Morning deliverable

- Branch with per-screen commits.
- `overnight_log.md` heartbeat history.
- Review packet: per-screen contact sheets (reference / before / after),
  family coverage ledger, deferred list with reasons.
- Updated UI docs: FriendslopStyle is the active lane, FlatStyle marked legacy.
