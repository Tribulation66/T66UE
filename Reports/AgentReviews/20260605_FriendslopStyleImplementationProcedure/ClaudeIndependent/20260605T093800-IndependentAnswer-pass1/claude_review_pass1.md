Result: OK

## Independent Answer

The task is authorable by Codex/Claude internally — it stops at a doc for user approval, and the user has already made the only user-level decisions (create a separate style; approve the doc later). Below are the concrete corrections and must-haves the central FriendslopStyle/FrenchFlop instruction file needs.

**1. Method class viability — YES, with a critical reframing.**
The repo already has the exact mechanism this style needs: `T66RuntimeUIBrushAccess.cpp` makes `ESlateBrushDrawType::Box` brushes with stored margins, `T66Style.cpp` documents the `9-slice: corners preserved, center stretches` behavior, and `T66ScreenSlateHelpers.cpp` has the left-cap/center-stretch/right-cap sliced button that overlays live content. So generated rubber raster chrome → N-slice brush → live Slate text overlay is mechanically supported today. **But** those same helpers (`ST66ReferenceHorizontalSlicedImage`, `MakeReferenceSlicedPlateButton`, etc.) are classified as *legacy chrome to be deleted* by the FlatStyle Step 0 cleanup regex (UI_FIDELITY_LOOP §5.2). FriendslopStyle must therefore own a **parallel namespace** (e.g. `FT66FriendslopStyle` + its own sliced-brush helpers/asset registry), and the doc must explicitly state that FriendslopStyle screens do **not** run FlatStyle's Step 0 legacy-chrome-removal as their cleanup target — that regex would destroy the very mechanism this style depends on.

**2. Pipeline the central file should require (reused process spine):**
- Step 0.5 reference geometry extraction → `UI/Geometry/<screen>_reference_geometry.md` + overlay via `Scripts/GenerateUIGeometryOverlay.py`, visually sanity-checked (breathing-room rule).
- Widget tagging convention + `T66.UI.DumpScreen`/`DumpWidget` structural dump.
- `Scripts/VerifyUIFidelity.py` against a 5-section checklist (Structure/Geometry/Colors/Content/Interactivity), with geometry copied from the corrected table.
- Pass log, iteration cap (5), stuck-FAIL-set escalation, ESCALATE → user-review packet.
- Multi-resolution responsive checks (1920×1080 is composition source, not runtime viewport).
- **New, style-specific steps:** (a) author each rubber chrome plate as a *fresh, clean, tileable/seamless* raster asset — never cropped from the mockup; (b) record per-asset N-slice margins; (c) wire via runtime texture/brush import as a `Box` brush; (d) keep all text/data as live Slate overlay.

**3. Reuse vs forbid:**
- *Reuse (process only):* geometry extraction, tagging, dump, VerifyUIFidelity, checklist format, pass log, termination/escalation, holistic contact-sheet review.
- *Forbid:* the FlatStyle *visual language* (pure Slate-native flat chrome, `bUseGlow=false` flat plates) as the target look; treating the FlatStyle "remove all raster chrome" rule as Friendslop's goal; importing the mockup itself as chrome; baking any label/score/player-data/localized text into raster (UI_AGENTS hard rule + Round06 manifest §7).

**4. Mockup → real UI without baked text or distortion:**
Decompose the mockup into chrome *elements* (button plate, panel plate, dropdown shell, divider), regenerate each as a standalone seamless raster with transparent padding, import as a `Box`/9-slice brush so corners stay fixed and only the center stretches, then place live `MakeFlatLabel`-equivalent Slate text/icons on top. Distortion is prevented by correct slice margins + sufficient source resolution; baked text is prevented by never generating text into the chrome asset.

**5. Anti-"lazy holistic" gate (the user's explicit concern):**
Add a mandatory **holistic gestalt gate** that runs after the per-item verifier passes and can independently FAIL even with zero per-item FAILs: a full-screen side-by-side (reference vs capture) judged as a whole for silhouette, weight/inflation, spacing rhythm, and overall resemblance. A screen is not DONE on checklist PASS alone — it must also pass this whole-screen sign-off (user-held for art direction, per the manifest's "final art-direction sign-off remains with the user").

## Evidence Checked
- `UI/FriendslopStyle/.../Round06/manifest.md` — mockup is **not** runtime chrome, text must stay live/localizable, output is 1672×941, art-direction sign-off is the user's.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` — full loop, Step 0/0.5, dump/verify infra, checklist/pass-log/termination, §5.2 legacy-chrome list (includes the slice helpers).
- `UI/UI_AGENTS.md` — hard rule banning generated raster chrome and routing through the two instruction docs.

## Questions Or Blockers
None requiring the user *before* the doc is authored. The doc itself is the approval artifact.

## Caveats
- **Governance conflict to surface in the doc, not silently ignore:** `UI/UI_AGENTS.md` hard rule "Do not use generated raster art for chrome" currently applies globally. The Friendslop doc must explicitly scope that ban to the FlatStyle path and declare a sanctioned carve-out for FriendslopStyle, otherwise the new doc contradicts repo law. Codex should flag this amendment for user approval as part of the approval package.
- New raster chrome assets need their own registry (analogous to `content_stubs_registry.md`/`icon_manifest.md`); define its path in the doc.
- Memory notes a prior **Slate brush texture import blocker** on the leaderboard port — runtime raster-brush import is a known risk area; the doc should call it out as a validation step before scaling to all screens.
- 1672×941 vs 1920×1080 normalization must be honored in geometry extraction and VerifyUIFidelity, per loop §10.2.
