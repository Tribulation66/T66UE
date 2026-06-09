# T66 UI Stage 2 Fresh Agent Handoff

## 1. Mission

Migrate the remaining eleven frontend screens to `FT66FlatStyle`, one screen at a time, following the UI Fidelity Loop until each screen is accepted. Do not migrate multiple screens in one session unless Pablo explicitly changes the scope.

Art-direction boundary: `ART_DIRECTION.md` makes FriendSlop canonical for 3D/world work. This handoff remains a 2D UI migration contract. `Settings Retro FX` is archived as of 2026-06-07 and should be skipped unless a later product decision restores it.

## 2. Required Reading Order

1. `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md` — canonical procedure and acceptance gate.
2. `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md` — master plan, palette, helper expectations, and per-screen specs.
3. `C:\UE\T66\Audit\Reference\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` — existing UI architecture and legacy chrome audit context.
4. `C:\UE\T66\UI\hero_selection_closeout_and_stage2_readiness.md` — Hero Selection closeout, open items, and Stage 2 lessons.
5. Hero Selection final artifacts as the template:
   - `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_report.md`
   - `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_contact_sheet.png`
   - `C:\UE\T66\UI\Checklists\hero_selection_checklist.md`
   - `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry.md`
   - `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry_overlay.png`
   - `C:\UE\T66\Saved\Codex\UI\HeroSelection\manual_interaction_checklist.md`
   - `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_log.md`
6. `C:\UE\T66\UI\Reference\UI_STAGE2_CAPTURE_READINESS_REFERENCE.md` — canonical capture names and parent-tab routing.

## 3. Migration Order

1. Overview
2. History
3. Diplomas
4. Drugs
5. Steam Achievements
7. Daily Descent
8. Challenges
9. Load Game
10. Run Summary

Rationale: this order starts with the most refined and simplest account screen, then migrates paired screens that reuse patterns, then moves into leaf screens and dense summary layouts. Hero Selection is already the Stage 1 pilot and should be used as the working example, not re-migrated.

## 4. Per-Screen Workflow Summary

For each screen, follow `UI_FIDELITY_LOOP_INSTRUCTIONS.md` exactly:

1. Run Step 0 legacy chrome cleanup against the screen's reachable code.
2. Run Step 0.5 reference geometry extraction from the V3 reference. Save the table to `C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md`.
3. Generate the overlay with:

```powershell
python C:\UE\T66\Scripts\GenerateUIGeometryOverlay.py --geometry C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md --output C:\UE\T66\UI\Geometry\<screen>_reference_geometry_overlay.png
```

4. Visually sanity-check the overlay and correct any too-tight or misleading geometry before implementation.
5. Build the screen with `FT66FlatStyle` chrome only. Tag every named element.
6. Generate missing icons/stubs via the M1 reference-region workflow and update the icon manifest.
7. Capture with `CaptureT66UIScreen.ps1` and `-T66AutoDumpScreen`.
8. Run `VerifyUIFidelity.py` and iterate until the automated report is clean.
9. Produce/update the manual interaction checklist and wait for Pablo's manual result before strict DONE.
10. Keep the pass log in the eight-heading section 7.1 structure.

## 5. Escalation Expectations

Keep iterating when the verifier reports objective FAIL items that are code-fixable: wrong geometry, wrong tag, wrong state, wrong color, wrong text, missing handler, or missing toggle metadata.

Stop and ask Pablo when:
- The iteration cap reaches 5.
- The FAIL set repeats unchanged after a fix attempt.
- The verifier is clean but UNSURE visual/content items remain.
- The reference and implementation disagree in a way that requires product/design choice.
- A missing backend or production asset blocks fidelity and cannot be stubbed under the content-stub policy.
- Manual interaction verification returns `Doesn't Work` and the expected behavior is ambiguous.

On ESCALATE, produce `Saved\Codex\UI\<ScreenName>\pablo_review.md` and stop.

## 6. Hero Selection Lessons

Legacy chrome leaks via `_Private` headers. Hero Selection showed that chrome dependencies can survive outside the main `BuildSlateUI()` file. Always audit screen-private headers and reachable helper files during Step 0, not just the primary screen class.

Walker composite-frame readback requires metadata override. Composite Slate frames may render correctly while dumping as transparent/black if the walker reads the wrong child. Use first-class `FT66FlatStyle` helpers with authoritative metadata for composite flat regions instead of adding one-off walker hacks.

Icon generation must use M1 reference-region workflow. Semantic prompts such as "male icon" produce reasonable but wrong icons. Crop the exact reference icon region, pass it as visual context, and prompt imagegen to reproduce the specific line weight, silhouette, fill, and details.

Labels vs buttons must be explicit. Non-interactive titles, section headers, subtitles, stat labels, stat values, and descriptions use `MakeFlatLabel` and must dump `is_label=true` with no visible border. Do not use button or panel helpers for plain text just because the typography looks similar.

Geometry tables need visual sanity checking. Raw tight bounding boxes can make the implementation look cramped. Draw the overlay, inspect the perceived regions, and include reference breathing room before treating the table as canonical.

Toggle groups need explicit `FT66FlatStyle` construction. Mutual exclusion should be represented by toggle-group metadata and state-driven rendering, not ad-hoc local border changes. The dump must show the correct `toggle_group` for every member.

Verifier passing is necessary but not sufficient. The automated report catches structural and measurable fidelity. Manual interaction remains the final gate, and any `Doesn't Work` result is a loop FAIL.

Capture script fails loudly on unknown screen names. Do not accept fallback captures. Every migrated screen must be resolvable by `-T66FrontendScreen`, including tab/category screens such as `Overview` and `Drugs`. `SettingsRetroFX` is archived and no longer part of the active migration list.

## 7. Pre-Rollout State Confirmation

- All eleven remaining Stage 2 screen names are accepted by the resolver. See `C:\UE\T66\UI\Reference\UI_STAGE2_CAPTURE_READINESS_REFERENCE.md`.
- Parent-tab/category routing exists for Overview, History, Diplomas, Drugs, Steam Achievements, and Load Game aliases. Settings Retro FX remains a legacy alias only.
- All eleven per-screen specs in master plan section 7.2 include Interactivity, label-vs-button, and icon manifest notes.
- The loop doc reflects Step 0, Step 0.5, M1 icon workflow, M2 label workflow, interactivity verification, manual interaction, resolver fail-loud behavior, pass log structure, content stub policy, termination conditions, and `-T66AutoDumpScreen`.
- `T66.UI.DumpScreen` and `VerifyUIFidelity.py` are reusable for tagged `FT66FlatStyle` screens.
- Geometry overlay generation is reusable via `Scripts\GenerateUIGeometryOverlay.py`.
- Full checklist auto-generation from spec + geometry table does not yet exist; Stage 2 can proceed with hand-authored checklists using Hero Selection as the template, or a future helper can be built before the first migration if Pablo wants checklist generation automated.

## 8. First Stage 2 Session

Start with **Overview**.

Use:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen Overview -Output C:\UE\T66\Saved\Codex\UI\Overview\pass_01_capture.png -DelaySeconds 5.5 -TimeoutSeconds 120 -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\Overview\pass_01_dump.json")
```

Before implementation, create:
- `C:\UE\T66\UI\Geometry\overview_reference_geometry.md`
- `C:\UE\T66\UI\Geometry\overview_reference_geometry_overlay.png`
- `C:\UE\T66\UI\Checklists\overview_checklist.md`
- `C:\UE\T66\Saved\Codex\UI\Overview\pass_log.md`

Do not begin History or any later screen until Overview is accepted or escalated.
