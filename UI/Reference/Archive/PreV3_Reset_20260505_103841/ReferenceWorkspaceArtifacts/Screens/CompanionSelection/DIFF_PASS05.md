# CompanionSelection Pass 05 Difference List

Reference:
- `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\CompanionSelection.png`

Packaged proof:
- `C:\UE\T66\UI\Reference\Screens\CompanionSelection\Proof\CompanionSelection_pass05_1672x941.png`

Accepted target-owned runtime assets:
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_cta_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_cta_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_cta_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_cta_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Buttons\companionselection_buttons_cta_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Panels\companionselection_panels_inner_panel_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\Panels\companionselection_panels_reference_scroll_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection\ScreenArt\companionselection_screen_art_center_scene_v1.png`

Current match:
- Center scene now uses the reference-matching companion scene with side pillars, routed through a CompanionSelection-owned screen art PNG.
- Side panels, top strip, carousel slots, skin rows, info panel, and CTA all route through CompanionSelection-owned assets or live Slate primitives.
- Button plates keep the sliced Reference path with nearest filtering, zero brush margin, live text, and normalized minimum widths.
- Live data remains live: skin names, currency value, selected companion name, lore, passive text, selected/equipped state, and CTA text are not baked into art.
- Packaged capture was taken from `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` after its timestamp advanced to `2026-05-03 23:03:16`.

Remaining differences:
- CTA plate shape is still the available target-owned rounded plate, while the reference has extra pointed side ornaments.
- Skin rows are still separate framed parchment rows; the reference reads more like a continuous parchment list with divider lines.
- Top strip and side panel chrome are close but not identical in edge ornament density.
- The right passive row is a live flat paper strip to avoid text clipping; it is less ornate than the reference but preserves live text.
