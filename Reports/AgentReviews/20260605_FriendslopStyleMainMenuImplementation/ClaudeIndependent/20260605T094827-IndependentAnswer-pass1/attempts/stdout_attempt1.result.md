Result: OK

## Independent Answer

This is implementable end-to-end without further user input; the user's go-ahead plus the approved instructions file cover the scope. Below are concrete paths, the minimal path, prerequisites, blockers, and the verify sequence.

**1. Files/code paths to inspect/edit**
- Router carve-out: `UI/UI_AGENTS.md` — the global ban lives at line 24 ("Do not use generated raster art for chrome…"). Scope it to FlatStyle and add a FriendslopStyle route to `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`. Also flip that file's header (`Status: Draft… pending user sign-off`, line 3) to approved, since the contract says the user signed off.
- Main Menu screen: `Source/T66/UI/Screens/T66MainMenuScreen.cpp` / `.h` and `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h`.
- Style plumbing: `Source/T66/UI/Style/` — add `FT66FriendslopStyle` (no Friendslop class exists yet; grep returned nothing). Reuse, don't extend as public API: `T66RuntimeUIBrushAccess.cpp` already has `MakeNineSliceBrush()` that sets `DrawAs=Box` with `FMargin` (lines 24–37, 423–434), and `T66RuntimeUITextureAccess.*` for texture loading. `T66ScreenSlateHelpers.*` and `T66FlatStyle.*` are reference patterns, not the target style.
- Scripts (all present): `Scripts/CaptureT66UIScreen.ps1`, `Scripts/GenerateUIGeometryOverlay.py`, `Scripts/VerifyUIFidelity.py`.

**2. Smallest viable path preserving the method class**
Carve out the router → write PPF/artifact-parity/mechanism gates in the pass log → fresh capture+dump of current Main Menu → geometry table+overlay from Round06 (native 1672x941, normalized to 1920x1080) → element manifest → author ONE chrome family (panel or primary button) → import and smoke-test a single 9-slice brush via the existing `MakeNineSliceBrush`/Box path before authoring the rest → build out `FT66FriendslopStyle` helpers → wire `T66MainMenuScreen` with live text/data + tags → capture/verify/iterate. Reusing the existing Box brush access (rather than a new slice engine) is what keeps the cost and method-class risk down.

**3. Artifacts that must exist before code edits**
The `UI/FriendslopStyle/` tree currently holds only the instructions file and `Reference/`. Missing and required by §4/§11: `Geometry/`, `Elements/`, `SliceSpecs/`, `Checklists/` (new Friendslop checklist — do NOT reuse the stale `UI/Checklists/main_menu_checklist.md`), `SourcePrompts/`, `friendslop_asset_registry.md`, `SourceAssets/UI/FriendslopStyle/MainMenu/`, plus the runtime chrome PNG families and the pre-import contact sheet. Pass log with PPF + Artifact Parity + Mechanism manifests must be written first.

**4. Likely major blockers justifying a stop**
- Chrome generation can't produce clean alpha/seam-safe slices → corners/highlights smear at min/wide widths (the §9 stretch test fails repeatedly).
- The "rubber" material read can't be achieved with sliced raster + Slate without a full-screen paintover (the exact thing prohibited) — this is the most probable approach-killer.
- Live leaderboard/friends/party data bindings missing for regions the reference shows as populated.
- Round06 native-resolution normalization mismatch corrupting geometry.

**5. Verification sequence before claiming success**
Compile (focused UI) → Unreal-owned capture + dump → `VerifyUIFidelity.py --checklist <friendslop_checklist> --dump --reference --capture --output --contact-sheet` (confirm current arg contract via `--help`) → holistic gestalt gate → responsive gate across the listed resolutions → manual interaction gate. Report PPF CLOSE + per-mechanism CLOSE; `FULL` only if every artifact/mechanism has current evidence, else `PARTIAL`.

**6. Unreal import/runtime + tooling warnings**
- Texture import: ensure non-Box fallback isn't silently triggered — `MakeNineSliceBrush` falls back to `DrawAs=Image` when margins are ~zero, which silently kills 9-slice; set nonzero normalized margins. Verify alpha/compression and filtering on import (matte removal must happen pre-import per §8).
- Tooling: don't call a screen passed if the verifier can't observe the issue (§11 Step I tooling-failure rule) — add dump tags/checklist support or document the manual gate instead.
- Treat the stale FlatStyle Main Menu checklist as invalid evidence.

## Evidence Checked
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (full).
- `UI/UI_AGENTS.md` (router; ban at line 24).
- `Source/T66/UI/Style/` listing; `T66RuntimeUIBrushAccess.cpp` Box/margin support; absence of any `Friendslop` class.
- Presence of all three Scripts; Main Menu screen source files; `UI/FriendslopStyle/` tree contents (only instructions + Reference present).

## Questions Or Blockers
None requiring the user — the go-ahead and approved procedure are sufficient. The "Draft/pending sign-off" header in the instructions file is a documentation lag, not a real gate; Codex should update it as part of the carve-out, not stop for it.

## Caveats
- I did not read the full body of `T66MainMenuScreen.cpp`, the verifier's current arg contract, or the Round06 manifest details — Codex should confirm the live-content list and exact `VerifyUIFidelity.py` flags at run time.
- The rubber-material-without-paintover risk (blocker #2) is real and may only surface after the first chrome family is authored; that is the most likely legitimate stop condition.
