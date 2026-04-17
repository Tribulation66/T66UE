# T66 WebLab

Browser-first UI lab for T66 screen mockups.

Purpose:
- Build each game screen as a local web prototype first.
- Lock visual direction, spacing, typography, and states before Unreal implementation.
- Use the approved browser version as the reference target for UMG/CommonUI work.

Recommended workflow:
1. Create one folder per screen under `screens/`.
2. Keep the screen prototype self-contained enough to preview locally.
3. Save reference images and inspiration in `references/`.
4. Move shared tokens, CSS, fonts, and reusable pieces into `shared/`.
5. Save approved screenshots or short captures in `exports/`.

Suggested per-screen structure:
- `screens/<screen-name>/index.html`
- `screens/<screen-name>/styles.css`
- `screens/<screen-name>/script.js`
- `screens/<screen-name>/notes.md`

Folder guide:
- `screens/` active browser pages for HUDs, menus, overlays, and other game screens
- `shared/` reusable styles, tokens, helper scripts, and local font wiring
- `references/` screenshots, paintovers, moodboards, and UI comparison captures
- `exports/` approved renders used as Unreal implementation targets

Notes:
- Keep designs local in the repo. No domain is required for this workflow.
- Prefer game-target layouts over fully responsive website layouts.
- Treat each approved screen here as the visual truth for its Unreal counterpart.
